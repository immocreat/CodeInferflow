#!/bin/bash
# Model: StarCoder2-3B
# StarCoder2-3B model is a 3B parameter model trained on 17 programming languages from The Stack v2, with opt-out requests excluded. The model uses Grouped Query Attention, a context window of 16,384 tokens with a sliding window attention of 4,096 tokens, and was trained using the Fill-in-the-Middle objective on 3+ trillion tokens.
# URL: https://huggingface.co/bigcode/starcoder2-3b

SRC_DIR=https://huggingface.co/bigcode/starcoder2-3b/resolve/main

FILE[0]=README.md
FILE[1]=config.json
FILE[2]=merges.txt
FILE[3]=model.safetensors
FILE[4]=special_tokens_map.json
FILE[5]=tokenizer.json
FILE[6]=tokenizer_config.json
FILE[7]=vocab.json

for i in {0..7}
do
  wget ${SRC_DIR}/${FILE[$i]}
done
