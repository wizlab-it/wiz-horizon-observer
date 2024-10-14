<?php
$payload = file_get_contents("php://input");
error_log(print_r($payload, 1) . "\n\n================\n\n", 3, "whoServer.log");

$json = json_decode($payload);
$image = explode(",", $json->messages[0]->content[1]->image_url->url, 2);
file_put_contents("whoServer.jpg", base64_decode($image[1]));

die('{
  "id": "whoServer-ADqpbNGz057uQ2eQ2bUK1Fe1tqmP0",
  "object": "chat.completion",
  "created": 1727864711,
  "model": "gpt-4o-2024-05-13",
  "choices": [
    {
      "index": 0,
      "message": {
        "role": "assistant",
        "content": "' . mt_rand(1, 4) . '",
        "refusal": null
      },
      "logprobs": null,
      "finish_reason": "stop"
    }
  ],
  "usage": {
    "prompt_tokens": 484,
    "completion_tokens": 1,
    "total_tokens": 485,
    "prompt_tokens_details": {
      "cached_tokens": 0
    },
    "completion_tokens_details": {
      "reasoning_tokens": 0
    }
  },
  "system_fingerprint": "fp_5796ac6771"
}');
