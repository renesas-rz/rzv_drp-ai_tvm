#
#  Original code(C) Copyright EdgeCortix, Inc. 2022
#  Modified Portion (C) Copyright Renesas Electronics Corporation 2022
#

from optparse import OptionParser
import os

def get_args():
    """
    Check argments and return option dictionaly data & input model file name
      Input : None (arguments)
      Output : option dictionary , AI model file path
    """
    usage = "usage: %prog [options] model_file"
    parser = OptionParser(usage)
    parser.set_defaults(drp_compiler_dir='./drp-ai_translator_release', toolchain_dir='/opt/poky/2.4.3', disable_concat = False)
    parser.add_option("-d", "--drp_compiler_dir", dest="drp_compiler_dir", help="DRP-AI Translator root directory", metavar="DIR")
    parser.add_option("-t", "--toolchain_dir", dest="toolchain_dir", help="Cross-compilation toolchain root directory", metavar="DIR")
    parser.add_option("-o", "--output_dir", dest="output_dir", help="Output directory", metavar="DIR") # Pattern 2
    parser.add_option("-s", "--input_shape", dest="input_shape", help="User specified input shape")
    parser.add_option("-i", "--input_name", dest="input_name", help="Input name")
    parser.add_option("-c", "--disable_concat", action="store_true", dest="disable_concat", default=False, help="Disable concat at DRP")
    parser.add_option("-q", "--fp16", action="store_true", dest="fp16", help="Convert to FP16")
    (options, args) = parser.parse_args()
    opts = vars(options)
    if len(args) != 1:
        parser.error("Expected a model file")

    print("[Check arguments]")
    model_file = str(args[0])
    print("  Input AI model         : ", model_file)
    
    # Set Cross-compiler path setting
    try: # From linux environment variable
        cc_path = os.environ["SDK"]
        opts["toolchain_dir"] = cc_path
        print("  SDK path               : ", opts["toolchain_dir"])
    except:
        print("  SDK path               : ", opts["toolchain_dir"], " (*default)")
    
    # Set DRP-AI Transaltor path setting
    try: # From linux environment variable
        drp_path = os.environ["TRANSLATOR"]
        opts["drp_compiler_dir"] = drp_path
        print("  DRP-AI Translator path : ", opts["drp_compiler_dir"])
    except:
        print("  DRP-AI Translator path : ", opts["drp_compiler_dir"], " (*default)")
    print("  Output dir             : ", opts["output_dir"])


    input_shape = opts["input_shape"]
    if input_shape is not None:
        input_shape = tuple([int(dim) for dim in input_shape.split(',')])
        print("  Input shape            : ", input_shape)
    else:
        input_shape = [1, 3, 224, 224]
        print("  Default input shape    : ", input_shape)
    opts["input_shape"] = input_shape
    
    return opts, model_file

if __name__ == "__main__":
    opts, model_file = args_set()