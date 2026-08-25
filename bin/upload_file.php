<?php

$filename = $_SERVER['HTTP_X_FILENAME'] ?? '';

if ($filename === '') {
    http_response_code(400);
    echo "Filename missing";
    exit;
}

$filename = basename($filename);

$file = __DIR__ . "/../files/" . $filename;

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

echo "Saved " . strlen($data) . " bytes";
?>