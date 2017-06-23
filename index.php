<!DOCTYPE HTML>
<html><head>

<meta http-equiv="refresh" content="30"> 
<style type="text/css">
@charset "UTF-8";
@import url(http://fonts.googleapis.com/css?family=Open+Sans:300,400,700);

body {
  font-family: 'Open Sans', sans-serif;
  font-weight: 300;
  line-height: 1em;
  color:#A7A1AE;
  background-image: url(bgpage.jpg);
  background-position: center center;
  background-repeat: no-repeat;
  background-attachment: fixed;
  background-size: cover;
  background-color:#333333;
}
.header img {
    display: block;
    margin: auto;
    width: 50%;

}


h1 {
  font-size:3em; 
  font-weight: 300;
  line-height:1em;
  text-align: center;
  color: #000000;
  
}

h2 {
  font-size:1em; 
  font-weight: 800;
  text-align: center;
  display: block;
  line-height:1em;
  padding-bottom: 2em;
  color: #66FF00;
}

h2 a {
  font-weight: 700;
  text-transform: uppercase;
  color: #FB667A;
  text-decoration: none;
}

.blue { color: #185875; }
.yellow { color: #FFF842; }

.container th h1 {
	  font-weight: bold;
	  font-size: 1em;
  text-align: center;
  color: #66FF00;
 
}

.container td {
	  font-weight: normal;
	  font-size: 1em;
  -webkit-box-shadow: 0 2px 2px -2px #0E1119;
	   -moz-box-shadow: 0 2px 2px -2px #0E1119;
	        box-shadow: 0 2px 2px -2px #0E1119;
}

.container {
	  text-align: left;
	  overflow: hidden;
	  width: 80%;
	  margin: 0 auto;
  display: table;
  padding: 0 0 2em 0;
}

.container td, .container th {
	  padding-bottom: 1%;
	  padding-top: 1%;
  padding-left:1%;  
  text-align:center;
}

/* Background-color of the odd rows */
.container tr:nth-child(odd) {
	  background-color: rgba(0,0,0,0.9);
	  
}

/* Background-color of the even rows */
.container tr:nth-child(even) {
	  background-color: rgba(0,0,0,0.6);
	  
}

.container th {
	  background-color: #333333;
}

.container td:first-child { color: #66FF00; }

@media (max-width: 800px) {
.container td:nth-child(4),
.container th:nth-child(4) { display: none; }
}
<</style>
<link rel="apple-touch-icon" href="/apple-icon-144x144px.png">
<link rel="apple-touch-icon" sizes="152x152" href="/touch-icon-ipad.png">
<link rel="apple-touch-icon" sizes="180x180" href="/touch-icon-iphone-retina.png">
<link rel="apple-touch-icon" sizes="167x167" href="/touch-icon-ipad-retina.png">
</head>
<body>

	<?php 
		require("connect.php");

		$sql2 = "SELECT * FROM `tempLog` ORDER BY `timeStamp` DESC LIMIT 1";
		$result = $conn->query($sql2);
		if ($result->num_rows > 0) {
			while($row = $result->fetch_assoc()) {
		
		$time = $row["timeStamp"];
		$temp1 = $row["coopTemp"];
		$temp2= $row["runTemp"];
		$doorStatus=$row["doorStatus"];
		$photocellStatus=$row["photocellStatus"];
		$heatStatus=$row["heatStatus"];
		$fanStatus=$row["fanStatus"];
		$lightStatus=$row["lightStatus"];
		    }
} else {
    echo "0 results";
}
$conn->close();
?>

 <div class="header">
  <img src="bgimage.png" alt="logo" />
</div>   

<table class="container">
	<tbody>
    <tr>
   <td>built by: James & Kaden Clark</td>
   </tr>
<tr>
<td><?php
  $dob = strtotime('05-03-2016');
$current_time = time();

$age_years = date('Y',$current_time) - date('Y',$dob);
$age_months = date('m',$current_time) - date('m',$dob);
$age_days = date('d',$current_time) - date('d',$dob);

if ($age_days<0) {
    $days_in_month = date('t',$current_time);
    $age_months--;
    $age_days= $days_in_month+$age_days;
}

if ($age_months<0) {
    $age_years--;
    $age_months = 12+$age_months;
}

echo "The girls are $age_years year, $age_months months, and $age_days days old.";
?></h2>
</td>
</tr>
</tbody>
</table>
<table class="container">
	<thead>
		<tr>
			<th><h1>Description</h1></h1></th>
            <th><h1>Values</h1></th>
            <th><h1>Set point</h1></th>
		</tr>
	</thead>
	<tbody>
		<tr>
			<td>TimeStamp</td>
            <td style="color:#33FF00"><?php echo $time; ?></td>
            <td>Time of last database query</td>
			
		</tr>
		<tr>
			<td>Coop Temperature</td>
            <td style="color:#33FF00"><?php echo $temp1; ?><sup>o</sup>F</td>
            <td>Current Inside Temperature</td>
			
		</tr>
		<tr>
			<td>Run Temperature</td>
            <td style="color:#33FF00"><?php echo $temp2; ?><sup>o</sup>F</td>
            <td>Current Ambient Temperature</td>
			
		</tr>
   		<tr>
			<td>Door</td>
            <td style="color:#33FF00"><?php echo $doorStatus; ?></td>
            <td>Open/Traveling/Closed</td>
			
		</tr>
        <tr>
			<td>Interior Light</td>
            <td style="color:#33FF00"><?php echo $lightStatus; ?></td>
            <td>On @ Twilight until door close</td>
			
		</tr>
        <tr>
			<td>Heater Relay</td>
            <td style="color:#33FF00"><?php echo $heatStatus; ?></td>
            <td>On @<= 40<sup>o</sup>F, Off @> 40<sup>o</sup>F</td>
			
		</tr>
        <tr>
			<td>Fan Relay</td>
            <td style="color:#33FF00"><?php echo $fanStatus; ?></td>
            <td>On @>= 90<sup>o</sup>F, Off @< 90<sup>o</sup>F</td>
			
		</tr>
        <tr>
			<td>Photocell</td>
            
            <td style="color:#33FF00">
			<?php 
			if (($photocellStatus >= "0") && ($photocellStatus <= "3")) {
    			echo "Dark";
			} elseif (($photocellStatus >= "4") && ($photocellStatus<= "120")) {
   			    echo "Twilight";
			} else if ($photocellStatus >= "125"){
    			echo "Light";
			}?></td>
            <td>Light/Twilight/Dark</td>
			
		</tr>
        <tr>
			<td>Water Level</td>
            <td style="color:#33FF00"><?php echo $fdp; ?></td>
            <td>Normal/Low</td>
			
		</tr>
        <tr>
			<td>Water Heater</td>
            <td style="color:#33FF00"><?php echo $fdp; ?></td>
            <td>On @<= 40<sup>o</sup>F, Off @> 40<sup>o</sup>F</td>
			
		</tr>
        <tr>
			<td>Feed Level</td>
            <td style="color:#33FF00"><?php echo $fdp; ?></td>
            <td>Normal/Low</td>
			
		</tr>
	</tbody>
	<?php ?>
</table>
</body>
<html>