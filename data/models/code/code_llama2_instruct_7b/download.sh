#!/bin/bash
# Model: Code-Llama2-Instruct-7B
SRC_DIR=https://huggingface.co/codellama/CodeLlama-7b-Instruct-hf/resolve/main

FILE[0]=LICENSE
FILE[1]=README.md
FILE[2]=config.json
FILE[3]=generation_config.json
FILE[4]=tokenizer.json
FILE[5]=tokenizer.model
FILE[6]=tokenizer_config.json
FILE[7]=special_tokens_map.json
FILE[8]=pytorch_model.bin.index.json
FILE[9]=pytorch_model-00001-of-00003.bin
FILE[10]=pytorch_model-00002-of-00003.bin
FILE[11]=pytorch_model-00003-of-00003.bin


for i in {0..11}
do
  wget ${SRC_DIR}/${FILE[$i]}
done
