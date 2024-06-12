: Model: CodeQwen1.5-7B

 @echo off
setlocal enabledelayedexpansion

set SRC_DIR=https://huggingface.co/Qwen/CodeQwen1.5-7B/resolve/main

set FILE[0]=LICENSE
set FILE[1]=README.md
set FILE[2]=config.json
set FILE[3]=tokenizer.json
set FILE[4]=tokenizer.model
set FILE[5]=tokenizer_config.json
set FILE[6]=model.safetensors.index.json
set FILE[7]=model-00001-of-00004.safetensors
set FILE[8]=model-00002-of-00004.safetensors
set FILE[9]=model-00003-of-00004.safetensors
set FILE[10]=model-00004-of-00004.safetensors

for /L %%i in (0,1,16) do (
    echo Downloading !FILE[%%i]! from %SRC_DIR%...
    call curl -L "%SRC_DIR%/!FILE[%%i]!" -o !FILE[%%i]!
)
