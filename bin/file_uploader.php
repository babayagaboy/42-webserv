<?php

$file = "./files/...";

$data = file_get_contents("php://stdin");

if ($data === false) {
    http_response_code(500);
    echo "Failed to read request body";
    exit;
}

if (file_put_contents($file, $data) === false) {
    http_response_code(500);
    echo "Failed to save image";
    exit;
}

header("Content-Type: text/plain");
echo "Image saved successfully";
?>