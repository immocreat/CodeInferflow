#!/bin/bash
# Model: CodeQwen1.5-7B

SRC_DIR=https://huggingface.co/Qwen/CodeQwen1.5-7B/resolve/main

FILE[0]=LICENSE
FILE[1]=README.md
FILE[2]=config.json
FILE[3]=tokenizer.json
FILE[4]=tokenizer.model
FILE[5]=tokenizer_config.json
FILE[6]=model.safetensors.index.json
FILE[7]=model-00001-of-00004.safetensors
FILE[8]=model-00002-of-00004.safetensors
FILE[9]=model-00003-of-00004.safetensors
FILE[10]=model-00004-of-00004.safetensors

for i in {0..10}
do
  url="${SRC_DIR}/${FILE[$i]}"
  wget $url
done
