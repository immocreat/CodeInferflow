# API

## Supported Format

- [Ollama Generate Format](#ollama-generate-format)
- [OpenAI Format](#openai-format)

## Ollama Generate Format

### Example

Simple request:
```bash
curl http://localhost:8080/api/generate -d '{
  "prompt": "def quicksort("
}'
```

Request with meta infomation:
```bash
curl http://localhost:8080/api/generate -d '{
  "prompt": "# Language: Python\n# Path: ~/code/sort.py\ndef quicksort( <SUF> ) <MID>"
}'
```
The language, path, code prefix and code suffix will be parsed to varibles `{code_language}`, `{code_file_path}`, `{code_prefix}` and `{code_suffix}`, respectively, which can be used in template.

Response:

```json
{
    "model": "default",
    "created_at": "2024-06-12T09:43:18",
    "response": "<code>",
    "input_tokens": 9,
    "output_tokens": 97,
    "total_duration": 2.149,
    "done": true
}
```

## OpenAI Format

Compatible with OpenAI API format. The stream mode is supported too. However, this format is mainly designed for chat model completion since the template is not defined for code completion.

### Example

Request: 

``` bash
curl -X post -d '{"model": "gpt-3.5-turbo","messages": [{"role": "system", "content": "You are a helpful assistant."}, {"role": "user", "content": "Write an article about the weather of Seattle"}], "stream": true}' http://localhost:8080/chat/completions
```

or use python:

```python
import openai

openai.base_url = "http://localhost:8080"
openai.api_key = "sk-no-key-required"

is_streaming = True

response = openai.chat.completions.create(
    model="default",
    messages=[
        {"role": "system", "content": "You are a helpful assistant."},
        {"role": "user", "content": "Write an article about the weather of Seattle."}
    ],
    stream = is_streaming
)

if is_streaming:
    for chunk in response:
        print(chunk.choices[0].delta.content or "", end = "")
else:
    print(response.choices[0].message.content)
```

Response:

```json
{
    "object": "chat.completion",
    "choices": [
        {
            "index": 0,
            "logprobs": null,
            "finish_reason": "stop",
            "message": {
                "content": "<contents>"
            }
        }
    ]
}
```
