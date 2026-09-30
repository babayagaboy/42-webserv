<?php
header("Content-Type: text/html; charset=UTF-8");
?>
<!DOCTYPE html>
<html>
<head><title>PHP CGI Test</title></head>
<body>
    <h1>PHP Dynamic Test</h1>
    <p>Current Time: <strong><?php echo date('Y-m-d H:i:s'); ?></strong></p>
    <p>Random Number: <strong><?php echo rand(1, 100); ?></strong></p>
</body>
</html>