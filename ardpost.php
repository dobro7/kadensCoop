<?php
/* instantiate our class, and select our database automatically */
$sql = new mysqli('yoursite.com','username','password','dbname');
 
/*
let's assume we've just received a form submission.
so we'll receive the information, and we'll escape it
*/
$temp1  = $sql->real_escape_string($_POST['temp1']);
$temp2  = $sql->real_escape_string($_POST['temp2']);
$doorStatus  = $sql->real_escape_string($_POST['doorStatus']);
$photocellStatus  = $sql->real_escape_string($_POST['photocellStatus']);
$heatStatus  = $sql->real_escape_string($_POST['heatStatus']);
$fanStatus  = $sql->real_escape_string($_POST['fanStatus']);
$lightStatus  = $sql->real_escape_string($_POST['lightStatus']);
 
/* build the query, we'll use an insert this time */
//change your dbname
$query = "INSERT INTO `dbname`.`tempLog` (`timeStamp`, `coopTemp`, `runTemp`, `doorStatus`, `photocellStatus`, `heatStatus`, `fanStatus`, `lightStatus`) VALUES (CURRENT_TIMESTAMP,'".$temp1."','".$temp2."', '".$doorStatus."','".$photocellStatus."','".$heatStatus."','".$fanStatus."','".$lightStatus."')";
 
/* execute the query, nice and simple */
$sql->query($query) or die($query.'<br />'.$sql->error);
 
?>
