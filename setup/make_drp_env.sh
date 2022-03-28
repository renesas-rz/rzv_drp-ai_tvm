
# Set symbolic link
ln -s ./3rdparty/dmlc-core ./tvm/3rdparty/dmlc-core
ln -s ./3rdparty/rang ./tvm/3rdparty/rang
ln -s ./3rdparty/vta-hw ./tvm/3rdparty/vta-hw
ln -s ./3rdparty/protobuf ./tvm/3rdparty/protobuf
ln -s ./3rdparty/libbacktrace ./tvm/3rdparty/libbacktrace
ln -s ./3rdparty/onnx ./tvm/3rdparty/onnx

# Set DRP-plugin python code
cp -r obj/python ./tvm/.
cp apps/transform.h ./tvm/include/tvm/relay/.

# Set libralies
cp -r ./obj/build ./tvm/.
cp -r ./obj/build_runtime ./tvm/.
