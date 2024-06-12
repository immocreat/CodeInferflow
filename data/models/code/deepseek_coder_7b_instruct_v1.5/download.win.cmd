: Model: deepseek-coder-7b-instruct-v1.5

 @echo off
setlocal enabledelayedexpansion

set SRC_DIR=https://huggingface.co/deepseek-ai/deepseek-coder-7b-instruct-v1.5/resolve/main

set FILE[0]=LICENSE
set FILE[1]=README.md
set FILE[2]=config.json
set FILE[3]=model-00001-of-00003.safetensors
set FILE[4]=model-00002-of-00003.safetensors
set FILE[5]=model-00003-of-00003.safetensors
set FILE[6]=model.safetensors.index.json
set FILE[7]=tokenizer.json
set FILE[8]=tokenizer_config.json


for /L %%i in (0,1,8) do (
    echo Downloading !FILE[%%i]! from %SRC_DIR%...
    call curl -L "%SRC_DIR%/!FILE[%%i]!" -o !FILE[%%i]!
)
