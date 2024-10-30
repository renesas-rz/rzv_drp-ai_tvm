import os
import sys
import argparse
from pathlib import Path
from collections import namedtuple
from os.path import join

import tensorflow._api.v2.compat.v1 as tf_1

sys.path.append("./repos/facenet_tf/src")
from models import inception_resnet_v1
from freeze_graph import main as freeze_graph


def save_train_disabled_model(ckpt_path, out_path):
    data_input = tf_1.placeholder(name="input", dtype=tf_1.float32, shape=[None, 160, 160, 3])
    output, _ = inception_resnet_v1.inference(
        data_input, keep_probability=0.8, phase_train=False, bottleneck_layer_size=512)
    label_batch= tf_1.identity(output, name="label_batch")
    embeddings = tf_1.identity(output, name="embeddings")

    init = tf_1.global_variables_initializer()
    with tf_1.Session() as sess:
        sess.run(init)
        saver = tf_1.train.Saver()
        saver.restore(sess, ckpt_path)
        save_path = saver.save(sess, out_path)

    return save_path

def read_pb_model(pb_model_path):
    with tf_1.gfile.GFile(pb_model_path, "rb") as f:
        graph_def = tf_1.GraphDef()
        graph_def.ParseFromString(f.read())
        return graph_def

def convert_pb_tflite(graph_def, export_path, input_name='input:0', output_name='output:0'):
    with tf_1.Session(graph=tf_1.Graph()) as sess:
        tf_1.import_graph_def(graph_def, name="")
        g = tf_1.get_default_graph()
        inp = g.get_tensor_by_name(input_name)
        out = g.get_tensor_by_name(output_name)

        # Initilize tflite converter.
        converter = tf_1.lite.TFLiteConverter.from_session(sess, [inp], [out])

        # Run converter.
        tflite_model = converter.convert()

        # Save tflite model to output dir.
        with open(export_path, 'wb') as f:
            f.write(tflite_model)

def main():
    tf_1.disable_v2_behavior()
    
    parser = argparse.ArgumentParser(description='Download FaceNet model and save it.')
    parser.add_argument("-n", "--model_name", dest="model_name", help="User specified model name.")
    args = parser.parse_args()

    model_name = args.model_name
    model_file = f'{model_name}.tflite'
    output_dir = join("output", f'{model_name}_tflite')
    out_path = join(output_dir, model_file)
    ckpt_path = "./FaceNet_Tensorflow/model-20180408-102900.ckpt-90"
    
    # Make output dir
    output_dir = join("output", f'{model_name}_tflite')
    os.makedirs(output_dir, exist_ok=True)
    
    # disable train specific layers
    tmp_inf_dir = Path("./tmp/inf_model")
    tmp_inf_dir.mkdir(parents=True)
    tmp_ckpt_path = tmp_inf_dir / "model-inference_facenet.ckpt-0"
    save_train_disabled_model(ckpt_path, str(tmp_ckpt_path))

    # freeze model state
    freezed_pb_path = "./tmp/frozen_model.pb"
    FreezeArgs = namedtuple("FreezeArgs", ["model_dir", "output_file"])
    freeze_graph(FreezeArgs(str(tmp_inf_dir), freezed_pb_path))

    # convert pb into tflite
    graph_def = read_pb_model(freezed_pb_path)
    convert_pb_tflite(graph_def, out_path, output_name="embeddings:0")

    print(f'{model_name} saved.')

if __name__ == "__main__":
    main()