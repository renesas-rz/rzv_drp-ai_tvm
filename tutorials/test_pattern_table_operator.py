#
# (C) Copyright EdgeCortix, Inc. 2021
#

import numpy as np
import onnx
import onnxruntime as rt

import tvm
from tvm import relay
from tvm.relay.transform import ToMixedPrecision
from tvm.relay.op.contrib.register import get_pattern_table


def get_relay_ir_drp_float16(mod):
    pass_list = tvm.transform.Sequential([
                    relay.transform.BackwardFoldScaleAxis(),
                    relay.transform.ForwardFoldScaleAxis(),
                    relay.transform.FoldConstant(),
                    relay.transform.DynamicToStatic(),
                    ToMixedPrecision("float16"),
                    relay.transform.FoldConstant(),  # remove cast operators
                    relay.transform.MergeComposite(get_pattern_table("mera_drp_no_concat")),
                    relay.transform.MergeComposite(get_pattern_table("mera_drp_only_concat")),
                ])
    float16_mod = pass_list(mod)
    relay_ir = float16_mod["main"].astext(show_meta_data=False)
    return str(relay_ir)


def test_conv2d():
    def get_relay_ir_conv2d(dshape, kshape, padding=(1, 1), groups=1, dilation=(1, 1), **attrs):
        x = relay.var("x", shape=dshape, dtype="float32")
        w = relay.const(np.random.random(size=kshape), dtype="float32")
        y = relay.nn.conv2d(x, w, padding=padding, dilation=dilation, groups=groups, **attrs)
        func = relay.Function([x], y)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    dshape = (1, 32, 18, 18)
    kshape = (64, 32, 3, 3)
    assert "Composite" in get_relay_ir_conv2d(dshape, kshape, padding=(1, 1), channels=64, groups=1, kernel_size=(3, 3))

    # supported: depthwise conv
    dshape = (1, 32, 18, 18)
    kshape = (32, 1, 3, 3)
    assert "Composite" in get_relay_ir_conv2d(dshape, kshape, padding=(1, 1), channels=32, groups=32, kernel_size=(3, 3))

    # not supported: plane size of feature map is too small
    dshape = (1, 32, 4, 4)
    kshape = (64, 32, 3, 3)
    assert "Composite" not in get_relay_ir_conv2d(dshape, kshape, padding=(1, 1), channels=64, groups=1, kernel_size=(3, 3))


def test_conv2d_bn_relu():
    def get_relay_ir_conv2d_bn_relu(dshape, kshape, padding=(1, 1), groups=1, dilation=(1, 1), **attrs):
        # conv
        x = relay.var("x", shape=dshape, dtype="float32")
        w = relay.const(np.random.random(size=kshape), dtype="float32")
        y = relay.nn.conv2d(x, w, padding=padding, dilation=dilation, groups=groups, **attrs)
        # bn
        beta = relay.const(np.random.random(size=[kshape[0]]), dtype="float32")
        gamma = relay.const(np.random.random(size=[kshape[0]]), dtype="float32")
        moving_mean = relay.const(np.random.random(size=[kshape[0]]), dtype="float32")
        moving_var = relay.const(np.random.random(size=[kshape[0]]), dtype="float32")
        y = relay.nn.batch_norm(y, gamma, beta, moving_mean, moving_var)
        # relu
        y = relay.nn.relu(y[0])
        func = relay.Function([x], y)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    dshape = (1, 32, 18, 18)
    kshape = (64, 32, 3, 3)
    assert "Composite" in get_relay_ir_conv2d_bn_relu(dshape, kshape, padding=(1, 1), \
        channels=64, groups=1, kernel_size=(3, 3))

    dshape = (1, 32, 18, 18)
    kshape = (32, 1, 3, 3)
    assert "Composite" in get_relay_ir_conv2d_bn_relu(dshape, kshape, padding=(1, 1), \
        channels=32, groups=32, kernel_size=(3, 3))


def test_add():
    def get_relay_ir_add(xshape, yshape):
        x = relay.var("x", shape=xshape, dtype="float32")
        y = relay.var("y", shape=yshape, dtype="float32")
        out = relay.add(x, y)
        func = relay.Function([x, y], out)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "Composite" in get_relay_ir_add([1, 3, 18, 18], [1, 3, 18, 18])

    # not supported: x and y have different sizes
    assert "Composite" not in get_relay_ir_add([1, 3, 18, 18], [10, 3, 18, 18])


# def test_bias_add():
#     for dtype in ["float16", "float32"]:
#         xshape = (10, 2, 3, 4)
#         bshape = (2,)
#         rtol = 1e-2 if dtype == "float16" else 1e-5
#         x = relay.var("x", shape=xshape, dtype=dtype)
#         bias = relay.var("bias", shape=bshape, dtype=dtype)
#         z = relay.nn.bias_add(x, bias)
#         func = relay.Function([x, bias], z)

#         x_data = np.random.uniform(size=xshape).astype(dtype)
#         y_data = np.random.uniform(size=bshape).astype(dtype)

#         verify_results(func, [x_data, y_data], "test_bias_add", rtol=rtol)


def test_dense():
    def get_relay_ir_dense(dshape, wshape):
        data = relay.var("data", shape=dshape, dtype="float32")
        weight = relay.const(np.random.random(size=wshape), dtype="float32")
        func = relay.Function([data], relay.nn.dense(data, weight))
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "Composite" in get_relay_ir_dense((1, 8), (16, 8))
    assert "Composite" in get_relay_ir_dense((1, 4), (3, 4))


def test_max_pool2d():
    def get_relay_ir_max_pool2d(x_shape, pool_size, strides, padding, ceil_mode):
        x = relay.var("x", relay.TensorType(x_shape, "float32"))
        y = tvm.relay.nn.max_pool2d(x, pool_size=pool_size, strides=strides, padding=padding, ceil_mode=ceil_mode)
        func = relay.Function([x], y)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "Composite" in get_relay_ir_max_pool2d((1, 4, 16, 16), pool_size=(2, 2), strides=(2, 2), \
        padding=(0, 0), ceil_mode=False)

    # not supported: ceil_mode should be False
    assert "Composite" not in get_relay_ir_max_pool2d((1, 4, 16, 16), pool_size=(2, 2), strides=(2, 2), \
        padding=(0, 0), ceil_mode=True)

    # not supported: strides can not be (3, 3)
    assert "Composite" not in get_relay_ir_max_pool2d((1, 4, 16, 16), pool_size=(3, 3), strides=(3, 3), \
        padding=(0, 0), ceil_mode=False)


def test_avg_pool2d():
    def get_relay_ir_avg_pool2d(x_shape, pool_size, strides, padding, ceil_mode):
        x = relay.var("x", relay.TensorType(x_shape, "float32"))
        y = tvm.relay.nn.avg_pool2d(x, pool_size=pool_size, strides=strides, padding=padding, ceil_mode=ceil_mode)
        func = relay.Function([x], y)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "Composite" in get_relay_ir_avg_pool2d((1, 4, 16, 16), pool_size=(2, 2), strides=(2, 2), \
        padding=(0, 0), ceil_mode=False)

    # not supported: ceil_mode should be False
    assert "Composite" not in get_relay_ir_avg_pool2d((1, 4, 16, 16), pool_size=(2, 2), strides=(2, 2), \
        padding=(0, 0), ceil_mode=True)

    # not supported: pool_size can not be (3, 3)
    assert "Composite" not in get_relay_ir_avg_pool2d((1, 4, 16, 16), pool_size=(3, 3), strides=(2, 2), \
        padding=(0, 0), ceil_mode=False)

    # not supported: strides can not be (3, 3)
    assert "Composite" not in get_relay_ir_avg_pool2d((1, 4, 16, 16), pool_size=(2, 2), strides=(3, 3), \
        padding=(0, 0), ceil_mode=False)

    # supported: GlobalAveragePool equivalent case
    assert "Composite" in get_relay_ir_avg_pool2d((1, 4, 16, 16), pool_size=(16, 16), strides=(16, 16), \
        padding=(0, 0), ceil_mode=False)


def test_global_avg_pool2d():
    def get_relay_ir_global_avg_pool2d(x_shape):
        x = relay.var("x", relay.TensorType(x_shape, "float32"))
        y = tvm.relay.nn.global_avg_pool2d(x)
        func = relay.Function([x], y)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "Composite" in get_relay_ir_global_avg_pool2d((1, 4, 16, 16))

    # not supported: data_c % 4 should be 0
    assert "Composite" not in get_relay_ir_global_avg_pool2d((1, 3, 16, 16))

    # not supported: data_c <= 16384 is not satisfied
    assert "Composite" not in get_relay_ir_global_avg_pool2d((1, 16388, 16, 16))


# def test_pad():
#     def verify_pad():
#         dshape = (4, 10, 7, 7)
#         x = relay.var("x", shape=dshape, dtype="int32")
#         y = relay.nn.pad(x, ((1, 1), (2, 2), (3, 3), (4, 4)))
#         func = relay.Function([x], y)
#         func = run_infer_type(func)
#         x_data = np.random.randint(low=-255, high=255, size=dshape).astype(np.int32)
#         verify_results(func, [x_data], "test_pad", rtol=1e-5, atol=1e-5)

#     verify_pad()


# def test_split():
#     def verify_split(dshape, indices_or_sections, axis=None):
#         dtype = "float32"
#         x = relay.var("x", relay.ty.TensorType(dshape, "float32"))
#         y = relay.split(x, indices_or_sections, axis=axis)
#         func = relay.Function([x], y.astuple())
#         x_data = np.random.uniform(size=dshape).astype(dtype)

#         verify_results(func, [x_data], "test_split", rtol=1e-5, atol=1e-5)

#     verify_split((5, 5, 2, 2), 5, axis=1)
#     verify_split((5, 5, 2, 2), 5, axis=0)
#     verify_split((5, 5, 2, 2), [1, 3, 4], axis=0)
#     verify_split((5, 5, 2, 2), [1, 3, 4], axis=1)


def test_concatenate():
    def case1():
        x = relay.var("x", relay.ty.TensorType((1,3,32,32), "float32"))
        y = relay.var("y", relay.ty.TensorType((1,3,32,32), "float32"))
        z = relay.var("z", relay.ty.TensorType((1,3,32,32), "float32"))
        in_vars = [x, y, z]
        inputs = [relay.add(x, x), relay.add(y, y), relay.add(z, z)]
        out_tensor = relay.concatenate(inputs, axis=1)
        func = relay.Function(in_vars, out_tensor)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    def case2():
        x = relay.var("x", relay.ty.TensorType((1,3,32,32), "float32"))
        y = relay.var("y", relay.ty.TensorType((1,3,32,32), "float32"))
        z = relay.var("z", relay.ty.TensorType((1,3,32,32), "float32"))
        in_vars = [x, y, z]
        inputs = [relay.add(x, x), relay.add(y, y), z]
        out_tensor = relay.concatenate(inputs, axis=1)
        func = relay.Function(in_vars, out_tensor)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "Composite=\"mera_drp.concatenate\"" in case1()
    assert "Composite=\"mera_drp.concatenate\"" not in case2()


def test_clip():
    def get_relay_ir_clip(dshape, a_min, a_max):
        x = relay.var("x", relay.TensorType(dshape, "float32"))
        y = relay.clip(x, a_min, a_max)
        func = relay.Function([x], y)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "Composite" in get_relay_ir_clip((5, 5, 2, 5), 0, 0.2)
    assert "Composite" in get_relay_ir_clip((5, 5, 2, 5), 0.2, 0.5)


# def test_lrn():
#     def verify_lrn(xshape, size, dtype="float32"):
#         x = relay.var("x", relay.ty.TensorType(xshape, dtype))
#         y = relay.nn.lrn(x, size=size, axis=1, alpha=1.0, beta=1.0, bias=1.0)
#         func = relay.Function([x], y)
#         x_data = np.random.uniform(size=xshape).astype(dtype)
#         verify_results(func, [x_data], "test_lrn", rtol=1e-5, atol=1e-5)

#     isize = [(1, 1, 480, 640), (1, 3, 224, 224)]
#     sizes = [1, 3]
#     for i in isize:
#         for s in sizes:
#             verify_lrn(i, s)


# def test_sigmoid():
#     def verify_sigmoid(dshape, dtype="float32"):
#         x = relay.var("x", relay.ty.TensorType(dshape, dtype))
#         y = relay.sigmoid(x)
#         func = relay.Function([x], y)
#         x_data = np.random.uniform(size=dshape).astype(dtype)
#         verify_results(func, [x_data], "test_sigmoid", rtol=1e-4, atol=1e-4)

#     isize = [(1, 3, 480, 640), (1, 3, 224, 224)]

#     for i in isize:
#         verify_sigmoid(i)


# def test_cast():
#     def verify_cast(dshape, dtype):
#         x = relay.var("x", relay.ty.TensorType(dshape, "float32"))
#         y = relay.cast(x, dtype)
#         func = relay.Function([x], y)
#         x_data = np.random.uniform(size=dshape).astype("float32")
#         verify_results(func, [x_data], "test_cast", rtol=1e-4, atol=1e-4)

#     isize = [(1, 3, 480, 640), (1, 3, 224, 224)]
#     out_dtypes = ["int8", "int16", "uint8", "uint16"]

#     for i in isize:
#         for o_dtype in out_dtypes:
#             verify_cast(i, o_dtype)


def test_reshape_dense():
    def get_relay_ir_reshape_dense(shape, newshape, wshape):
        data = relay.var("data", relay.TensorType(shape, "float32"))
        out = relay.reshape(data, newshape=newshape)
        weight = relay.const(np.random.random(size=wshape), dtype="float32")
        out = relay.nn.dense(out, weight)
        func = relay.Function([data], out)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "Composite=\"mera_drp.reshape_or_flatten_and_dense\"" in \
        get_relay_ir_reshape_dense((1, 2, 3, 4), (1, 24), (3, 24))


if __name__ == "__main__":
    # single operator
    test_conv2d()
    test_conv2d_bn_relu()
    test_add()
    # test_bias_add()
    test_dense()
    test_max_pool2d()
    # test_avg_pool2d()
    test_global_avg_pool2d()
    # test_pad()
    # test_split()
    test_concatenate()
    test_clip()
    # test_lrn()
    # test_sigmoid()
    # test_cast()
    # test_cast()

    # sequence of operators
    test_reshape_dense()
