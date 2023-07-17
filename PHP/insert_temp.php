<?php
echo "---------------------------------------------------------------------------------PHP\n";
include 'secrets.php';
$lastInsertId;
if (isset($_GET["temp_value"], $_GET["temp_date"], $_GET["lat"], $_GET["lon"])) {
    // Get values from HTTP GET
    $temp_value = $_GET["temp_value"];
    $temp_date = $_GET["temp_date"];
    $lat = $_GET["lat"];
    $lon = $_GET["lon"];

    // Create connection
    $conn = new mysqli($servername, $username, $password, $dbname);
    // Check connection
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

	function insertToDB($connection, $query){
		if ($query->execute() === TRUE) {
			// echo "New record created successfully\n";
			// echo "Inserted: \n";
			// echo $temp_value . "\n";
			// echo $temp_date . "\n";
			// echo $lat . "\n";
			// echo $lon . "\n\n\n";
			$lastInsertId = $connection->insert_id;
			echo "ID of last inserted record is: $lastInsertId \n";
			return $lastInsertId;
		} else {
			echo "Error: " . $query->error;
			return null;
		}
	}

	include('get_forecast.php'); //get data from weather API
    $stmt = $conn->prepare("INSERT INTO arduino (temp_value, temp_date, latitude, longitude) VALUES (?, ?, ?, ?)"); //prepare Arduino SQL statement
    $stmt->bind_param('dsdd', $temp_value, $temp_date, $lat, $lon);
	//DEBUG
	// $sql_query = $stmt->sqlstate;
	// echo "\n $sql_query \n";
	//

    $lastInsertId = insertToDB($conn, $stmt); //insert Arduino data to DB

	$stmt = $conn->prepare("INSERT INTO `forecast` (forecast_value, arduino_id) VALUES (?, ?)"); //prepare Weather SQL statement
	$stmt->bind_param('di', $tempWeatherAPI, $lastInsertId);
	echo "tempWeatherAPI: $tempWeatherAPI, lastInsertId: $lastInsertId \n";
	//DEBUG
	// $sql_query = $stmt->sqlstate;
	// echo "\n $sql_query \n";
	//
	insertToDB($conn, $stmt); //insert Weather data to DB
    
    $conn->close();
} else {
    echo "data is not set";
}
?>
