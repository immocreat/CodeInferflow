: Model: StarCoder2-3B

 @echo off
setlocal enabledelayedexpansion

set SRC_DIR=https://huggingface.co/bigcode/starcoder2-3b/resolve/main

set FILE[0]=README.md
set FILE[1]=config.json
set FILE[2]=merges.txt
set FILE[3]=model.safetensors
set FILE[4]=special_tokens_map.json
set FILE[5]=tokenizer.json
set FILE[6]=tokenizer_config.json
set FILE[7]=vocab.json


for /L %%i in (0,1,7) do (
    echo Downloading !FILE[%%i]! from %SRC_DIR%...
    call curl -L "%SRC_DIR%/!FILE[%%i]!" -o !FILE[%%i]!
)
