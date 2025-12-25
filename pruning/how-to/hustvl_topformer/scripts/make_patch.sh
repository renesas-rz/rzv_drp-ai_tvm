# !/bin/sh
set -eux

SCRIPT_FILE_DIR=`dirname ${0}`
MODIFIED_FILES_DIR=$(cd ${SCRIPT_FILE_DIR}/../; pwd)
REFERENCE_FILES_DIR=$(cd "$MODIFIED_FILES_DIR"; pwd)
cd $REFERENCE_FILES_DIR

OUTPUT_FILES_DIR=$MODIFIED_FILES_DIR/patch

# 出力ディレクトリが存在すれば削除、なければ作成する。
if [ -d $OUTPUT_FILES_DIR ]; then
    rm -r $OUTPUT_FILES_DIR
fi
mkdir -p $OUTPUT_FILES_DIR


# 参照ファイルとの差分を用いてパッチを作成する
function make_patch_w_reffile(){
    local ref_filepath=$1           # 参照ファイル       absolute path
    local mod_filepath=$2           # 変更後のファイル   absolute path
    local output_patch_filepath=$3  # 出力パッチファイル absolute path

    # 参照ファイルのディレクトリを取得
    local dir_of_ref_file=$(cd $(dirname $ref_filepath); pwd)

    # 変更後のファイルを参照ファイルと同じディレクトリにコピー
    cp $mod_filepath $dir_of_ref_file
    local mod_filepath_on_refdir=$dir_of_ref_file/$(basename $mod_filepath)

    # カレントディレクトリ（REFERENCE_FILES_DIR)からの相対パスを取得
    local rel_ref_filepath=$(realpath --relative-to=. $ref_filepath)
    local rel_mod_filepath_on_refdir=$(realpath --relative-to=. $mod_filepath_on_refdir)

    # パッチを作成する
    git diff --no-index --no-prefix -u $rel_ref_filepath $rel_mod_filepath_on_refdir > $output_patch_filepath || true
    rm $rel_mod_filepath_on_refdir
}

# 新規に追加するファイルのパッチを作成する。
function make_patch_w_devnull(){
    local ref_dir=$1                # 変更後のファイルをどこに置くかのディレクトリ
    local mod_filepath=$2           # 変更後のファイル   absolute path
    local output_patch_filepath=$3  # 出力パッチファイル absolute path

    # 参照ファイルのディレクトリを取得
    local dir_of_ref_file=$(cd $ref_dir; pwd)

    # 変更後のファイルを参照ファイルと同じディレクトリにコピー
    cp $mod_filepath $dir_of_ref_file
    local mod_filepath_on_refdir=$dir_of_ref_file/$(basename $mod_filepath)

    # カレントディレクトリ（REFERENCE_FILES_DIR)からの相対パスを取得
    local rel_mod_filepath_on_refdir=$(realpath --relative-to=. $mod_filepath_on_refdir)

    # パッチを作成する
    git diff --no-index --no-prefix -u /dev/null $rel_mod_filepath_on_refdir > $output_patch_filepath || true
    rm $rel_mod_filepath_on_refdir
}

make_patch_w_reffile $REFERENCE_FILES_DIR/TopFormer/tools/deploy_test.py \
                     $MODIFIED_FILES_DIR/onnx_validate.py \
                     $OUTPUT_FILES_DIR/onnx_validate.py.patch

make_patch_w_reffile $REFERENCE_FILES_DIR/TopFormer/tools/convert2onnx.py \
                     $MODIFIED_FILES_DIR/torch2onnx.py \
                     $OUTPUT_FILES_DIR/torch2onnx.py.patch

make_patch_w_reffile $REFERENCE_FILES_DIR/TopFormer/tools/test.py \
                    $MODIFIED_FILES_DIR/eval_with_pruned_model.py \
                    $OUTPUT_FILES_DIR/eval_with_pruned_model.py.patch

make_patch_w_reffile $REFERENCE_FILES_DIR/TopFormer/local_configs/topformer/topformer_base_512x512_160k_2x8_ade20k.py \
                     $MODIFIED_FILES_DIR/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py \
                     $OUTPUT_FILES_DIR/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py.patch

make_patch_w_reffile $REFERENCE_FILES_DIR/TopFormer/local_configs/topformer/topformer_base.py \
                     $MODIFIED_FILES_DIR/modified_topformer_base.py \
                     $OUTPUT_FILES_DIR/topformer_base.py.patch

make_patch_w_devnull $REFERENCE_FILES_DIR/TopFormer/mmcv/mmcv/runner/hooks/ \
                    $MODIFIED_FILES_DIR/pruning_hook.py \
                    $OUTPUT_FILES_DIR/pruning_hook.py.patch