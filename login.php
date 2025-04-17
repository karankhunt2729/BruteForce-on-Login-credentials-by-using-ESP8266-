<?php
session_start();

// Function to generate math CAPTCHA
function generateCaptcha() {
    $num1 = rand(1, 9);
    $num2 = rand(1, 9);
    $operators = ['+', '-', '*'];
    $op = $operators[array_rand($operators)];
    $_SESSION['captcha_answer'] = eval("return $num1 $op $num2;");
    return "$num1 $op $num2";
}

// If form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $username = $_POST['username'] ?? '';
    $password = $_POST['password'] ?? '';
    $captcha = $_POST['captcha'] ?? '';

    // Check CAPTCHA
    if (!isset($_SESSION['captcha_answer']) || intval($captcha) !== $_SESSION['captcha_answer']) {
        echo "❌ CAPTCHA Failed!";
        exit;
    }

    // Database connection
    $conn = new mysqli("localhost", "root", "", "login_db");
    if ($conn->connect_error) {
        die("❌ Database Connection Failed: " . $conn->connect_error);
    }

    // Validate credentials
    $stmt = $conn->prepare("SELECT * FROM users WHERE username = ? AND password = ?");
    $stmt->bind_param("ss", $username, $password);
    $stmt->execute();
    $result = $stmt->get_result();

    if ($result->num_rows > 0) {
        echo "✅ Login Successful!";
    } else {
        echo "❌ Invalid username or password!";
    }

    $stmt->close();
    $conn->close();
    exit;
}

// On GET request — Show login page with new CAPTCHA
$captcha_question = generateCaptcha();
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Login Page with CAPTCHA</title>
</head>
<body>
    <h2>🔐 Secure Login</h2>
    <form action="" method="post">
        Username: <input type="text" name="username" required><br><br>
        Password: <input type="password" name="password" required><br><br>
        CAPTCHA: <?php echo $captcha_question; ?><br>
        <input type="text" name="captcha" required><br><br>
        <input type="submit" value="Login">
    </form>
</body>
</html>
