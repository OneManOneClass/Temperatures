<?php
include 'secrets.php';

// URL for the API request
$url = "https://api.openweathermap.org/data/2.5/weather?lat=$lat&lon=$lon&appid=$openweathermapAPIKey&units=metric";
// echo "$url \n";

$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);

$response = curl_exec($ch);

// echo "<br>";
// echo $url;
// echo "<br>";


if (curl_errno($ch)) {
    $error_msg = curl_error($ch);
}

curl_close($ch);

// Decode the JSON response
$data = json_decode($response, true);

// Check if the JSON decoding was successful
if ($data === null) {
    echo "Error: Unable to parse JSON response.\n";
    exit;
}

if (isset($data['main']['temp'])) {
    $tempWeatherAPI = $data['main']['temp'];
    echo "Temperature: $tempWeatherAPI Celcius <br>";
} else {
    echo "Error: 'temp' value not found in the response.\n";
}
?>
