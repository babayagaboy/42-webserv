<?php

// Ordinary HTML upload forms send multipart/form-data.
$contentType = $_SERVER['CONTENT_TYPE'] ?? '';

if (strpos($contentType, 'multipart/form-data') === 0) {
    $upload = $_FILES['file'] ?? null;

    if ($upload === null || $upload['error'] !== UPLOAD_ERR_OK) {
        http_response_code(400);
        header('Content-Type: text/plain');
        echo 'Upload failed. Choose a file within the PHP upload size limit.';
        exit;
    }

    $filename = basename($upload['name']);
    $directory = dirname(__DIR__) . '/files/';

    if ($filename === '' || $filename === '.' || $filename === '..') {
        http_response_code(400);
        header('Content-Type: text/plain');
        echo 'Invalid filename';
        exit;
    }

    if (!move_uploaded_file($upload['tmp_name'], $directory . $filename)) {
        http_response_code(500);
        header('Content-Type: text/plain');
        echo 'Could not save the uploaded file';
        exit;
    }

    http_response_code(201);
    header('Content-Type: text/plain');
    echo 'File successfully uploaded. Open Files to view it.';
    exit;
}

// Keep raw uploads compatible with the existing HTTP tests and older demo.
$filename = $_SERVER['HTTP_X_FILENAME'] ?? '';

if ($filename === '') {
    http_response_code(400);
    echo "Filename missing";
    exit;
}

// Some HTTP clients percent-encode the filename header.
if (($_SERVER['HTTP_X_FILENAME_ENCODING'] ?? '') === 'percent') {
    $filename = rawurldecode($filename);
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