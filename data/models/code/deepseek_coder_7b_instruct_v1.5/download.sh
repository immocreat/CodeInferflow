#!/bin/bash
# Model: deepseek-coder-7b-instruct-v1.5
SRC_DIR=https://huggingface.co/deepseek-ai/deepseek-coder-7b-instruct-v1.5/resolve/main

FILE[0]=LICENSE
FILE[1]=README.md
FILE[2]=config.json
FILE[3]=model-00001-of-00003.safetensors
FILE[4]=model-00002-of-00003.safetensors
FILE[5]=model-00003-of-00003.safetensors
FILE[6]=model.safetensors.index.json
FILE[7]=tokenizer.json
FILE[8]=tokenizer_config.json

for i in {0..8}
do
  wget ${SRC_DIR}/${FILE[$i]}
done
