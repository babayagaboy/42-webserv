<?php

$filename = $_SERVER['HTTP_X_FILENAME'] ?? '';

if ($filename === '') {
    http_response_code(400);
    echo "Filename missing";
    exit;
}

$filename = basename($filename);

$uploadDirectory = dirname(__DIR__) . DIRECTORY_SEPARATOR . 'files';

$file = rtrim($uploadDirectory, DIRECTORY_SEPARATOR)
    . DIRECTORY_SEPARATOR . $filename;

$data = file_get_contents("php://input");

if ($data === false) {
    http_response_code(500);
    echo "Failed to read request body";
    exit;
}

if (file_put_contents($file, $data) === false) {
    http_response_code(500);
    echo "Failed to save file";
    exit;
}

http_response_code(201);
echo "Saved " . strlen($data) . " bytes";
?>