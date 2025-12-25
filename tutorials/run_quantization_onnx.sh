if [[ $# -ne 6 ]]; then
  echo "${0} expected 6 parameters:"
  echo "  <drp quantization tool>"
  echo "  <model data directory>"
  echo "  <calibration data directory>"
  echo "  <input model dir>"
  echo "  <output onnx model>"
  echo "  <quantization option>"
  exit 1
fi

if [ ! -d "$1" ]; then
  echo "${0} first parameter: Quatization tool $1 not found"
  exit 1
fi

if [ ! -d "$2" ]; then
  echo "${0} second parameter: Input model network $2 not found"
  exit 1
fi

if [ ! -d "$3" ]; then
  echo "${0} third parameter: Calibration data not found"
  exit 1
fi

TOOL_DIR=$1
MODEL_DIR=$2
CALIBRATION_DIR=$3
INPUT=$4
OUTPUT=$5
QUANT_OPTION=$6

if [ ! -d ${CALIBRATION_DIR}/${INPUT}/input ]; then
  echo "Calibration input network data not found"
  exit 1
fi

if [ ! -d ${CALIBRATION_DIR}/${INPUT}/output ]; then
  echo "Calibration output network data not found"
  exit 1
fi

cp -r ${CALIBRATION_DIR}/${INPUT}/input ${MODEL_DIR}/${INPUT}
cp -r ${CALIBRATION_DIR}/${INPUT}/output ${MODEL_DIR}/${INPUT}

if [ -z $OPTIMIZER_ENABLE ] ; then
  PREOPTIMIZE=${MODEL_DIR}/${INPUT}/pre_optimize_model.onnx
  mv ${MODEL_DIR}/${INPUT}/model.onnx $PREOPTIMIZE
  python3 ${TOOL_DIR}/../translator/DRP-AI_Translator/onnx_optimizer/run_onnx_optimizer.py\
    --file_in $PREOPTIMIZE --file_out ${MODEL_DIR}/${INPUT}/model.onnx
else
  if "${OPTIMIZER_ENABLE}" ; then
    PREOPTIMIZE=${MODEL_DIR}/${INPUT}/pre_optimize_model.onnx
    mv ${MODEL_DIR}/${INPUT}/model.onnx $PREOPTIMIZE
    python3 ${TOOL_DIR}/../translator/DRP-AI_Translator/onnx_optimizer/run_onnx_optimizer.py\
      --file_in $PREOPTIMIZE --file_out ${MODEL_DIR}/${INPUT}/model.onnx
  fi
fi

work_dir=$PWD
cd ${TOOL_DIR}
python3 -m drpai_quantizer.cli_interface --calibrate_method MinMax \
        --input_model_path ${work_dir}/${MODEL_DIR}/${INPUT} \
        --output_model_path ${work_dir}/${OUTPUT} --tvm \
        ${QUANT_OPTION} \
        --opts exclude_act_func_dir ./exclude_operator


RET_CODE=$?
if [ ${RET_CODE} -ne 0 ]; then
  echo "Error: drpai_quantize.py failed"
  if [ ! -e ${work_dir}/${OUTPUT} ]; then
    exit ${RET_CODE}
  fi
  echo "However, the output file exists! The quantization process continued."
fi

cd ${work_dir}
rm -rf ${MODEL_DIR}/${INPUT}/input
rm -rf ${MODEL_DIR}/${INPUT}/output
