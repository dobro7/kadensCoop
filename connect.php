<?php

$servername = "yoursite.com"; //change this
$username = "username"; //change this
$password = "password"; //change this
$dbname = "dbname"; //change this
// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

		return $connection;
		
?>