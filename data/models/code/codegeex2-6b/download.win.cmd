: Model: codegeex2-6b

 @echo off
setlocal enabledelayedexpansion

set SRC_DIR=https://huggingface.co/THUDM/codegeex2-6b/resolve/main

set FILE[0]=MODEL_LICENSE
set FILE[1]=README.md
set FILE[2]=config.json
set FILE[3]=configuration_chatglm.py
set FILE[4]=modeling_chatglm.py
set FILE[5]=quantization.py
set FILE[6]=tokenization_chatglm.py
set FILE[7]=tokenizer.model
set FILE[8]=tokenizer_config.json
set FILE[9]=pytorch_model.bin.index.json
set FILE[10]=pytorch_model-00001-of-00007.bin
set FILE[11]=pytorch_model-00002-of-00007.bin
set FILE[12]=pytorch_model-00003-of-00007.bin
set FILE[13]=pytorch_model-00004-of-00007.bin
set FILE[14]=pytorch_model-00005-of-00007.bin
set FILE[15]=pytorch_model-00006-of-00007.bin
set FILE[16]=pytorch_model-00007-of-00007.bin

for /L %%i in (0,1,16) do (
    echo Downloading !FILE[%%i]! from %SRC_DIR%...
    call curl -L "%SRC_DIR%/!FILE[%%i]!" -o !FILE[%%i]!
)
