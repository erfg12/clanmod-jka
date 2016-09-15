<?PHP
//usage: key=OurKey&p=Purpose&OTHER_DATA
$servername = "localhost";
$username = "myDatabaseUsername";
$password = "myDatabasePassword";
$dbname = "clanmod";

$key = "ourSecretKey"; //our key to prove it's us sending this post

if (!stristr($_POST['key'],$key)) //incorrect key = no response
	return;

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error)
    die("Connection failed: " . $conn->connect_error);

if (strstr($_POST['p'],"register")){
	$sql = "INSERT INTO users (username, password, ipaddress) VALUES ('$_POST[user]', '$_POST[pass]', '$_POST[ipaddress]')";
	if ($conn->query($sql) === TRUE){
		$insertedID = $conn->insert_id;
    	$sql2 = "INSERT INTO jedi_academy (user_id) VALUES ('$insertedID')";
		if ($conn->query($sql2) === TRUE){
    		echo "successful";
		} else
			echo "Error: " . $sql . " " . $conn->error;
	} else
		echo "Error: " . $sql . " " . $conn->error;
}

if (strstr($_POST['p'],"login")){
	$sql = "SELECT * FROM users WHERE username = '$_POST[user]' AND password = '$_POST[pass]'";
	$result = $conn->query($sql);

	if ($result->num_rows > 0) {
    	while($row = $result->fetch_assoc()) {
			echo $row["id"];
    	}
	} else
    	echo "0";
}

if (strstr($_POST['p'],"find")){
	$sql = "SELECT * FROM users WHERE username = '$_POST[user]'";
	$result = $conn->query($sql);

	if ($result->num_rows > 0) {
    	while($row = $result->fetch_assoc()) {
			echo $row["id"];
    	}
	} else
    	echo "0";
}

if (strstr($_POST['p'],"stats")){
	$sql = "SELECT * FROM ". $_POST['g'] ." WHERE user_id = '$_POST[id]'";
	$result = $conn->query($sql);
	$array = array();
	
	$numFields = $result->field_count;

	if ($result->num_rows > 0) {
    	while($row = $result->fetch_array ()) {
			for ($i=0;$i<$numFields;$i++){
				if ($i == 0)
					continue;
					
				array_push($array, $row[$i]);
			}
    	}
		echo implode (",",$array);
	} else
    	echo "0";
}

if (strstr($_POST['p'],"leaders")){ //outputs 3 integers divided by a semicolon
	$sql = "SELECT ". $_POST['r'] ." FROM ". $_POST['g'] ." ORDER BY ". $_POST['o'] ." DESC LIMIT 5";
	$result = $conn->query($sql);
	$arrayC = array();
	
	$numFields = $result->field_count;

	if ($result->num_rows > 0) {
    	while($row = $result->fetch_array ()) {
			$array = array();
			for ($i=0;$i<$numFields;$i++){
				if ($i == 0){
					$result2 = $conn->query("SELECT username FROM users WHERE id = '$row[0]'");
					$userRow = $result2->fetch_row();
					array_push($array, $userRow[0]);
				} else
					array_push($array, $row[$i]);
			}
			$imp = implode (",",$array);
			array_push($arrayC, $imp);
			unset($array);
    	}
		echo implode (";",$arrayC);
	} else
    	echo "0";
}

if (strstr($_POST['p'],"increase")){
	$sql = "UPDATE ". $_POST['g'] ." SET ". $_POST['c'] ." = ". $_POST['c'] ." + 1 WHERE id = '$_POST[id]'";
	if ($conn->query($sql) === TRUE){
    	echo "successful"; 
	} else
		echo "Error: " . $sql . " " . $conn->error;
}

$conn->close();
?>