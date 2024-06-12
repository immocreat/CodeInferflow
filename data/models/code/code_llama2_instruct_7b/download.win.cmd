: Model: Code-Llama2-Instruct-7B

 @echo off
setlocal enabledelayedexpansion

set SRC_DIR=https://huggingface.co/codellama/CodeLlama-7b-Instruct-hf/resolve/main

set FILE[0]=LICENSE
set FILE[1]=README.md
set FILE[2]=config.json
set FILE[3]=generation_config.json
set FILE[4]=tokenizer.json
set FILE[5]=tokenizer.model
set FILE[6]=tokenizer_config.json
set FILE[7]=special_tokens_map.json
set FILE[8]=pytorch_model.bin.index.json
set FILE[9]=pytorch_model-00001-of-00003.bin
set FILE[10]=pytorch_model-00002-of-00003.bin
set FILE[11]=pytorch_model-00003-of-00003.bin

for /L %%i in (0,1,11) do (
    echo Downloading !FILE[%%i]! from %SRC_DIR%...
    call curl -L "%SRC_DIR%/!FILE[%%i]!" -o !FILE[%%i]!
)
