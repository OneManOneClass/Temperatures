<?php

if(isset($_GET["temp_value"],$_GET["temp_date"])) {
   $temp_value = $_GET["temp_value"]; // get temperature value from HTTP GET
   $temp_date = $_GET["temp_date"]; // get temp_date value from HTTP GET

   $servername = "localhost";
   $username = "Arduino";
   $password = "arduino";
   $dbname = "db_arduino";

   // Create connection
   $conn = new mysqli($servername, $username, $password, $dbname);
   // Check connection
   if ($conn->connect_error) {
      die("Connection failed: " . $conn->connect_error);
   }

	$stmt = $conn->prepare("INSERT INTO tbl_temp (temp_value, temp_date) VALUES (?, ?)");
	$stmt->bind_param('ds', $temp_value, $temp_date);
	echo "Inserted: \n";
	echo $temp_value;
	echo "\n";
	echo $temp_date;
	echo "\n\n\n";


   if ($stmt->execute() === TRUE) { 
      echo "New record created successfully";
   } else {
      echo "Error: " . $sql . " => " . $conn->error;
   }

   $conn->close();
} else {
   echo "data is not set";
}
?>