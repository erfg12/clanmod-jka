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
	$sql = "INSERT INTO users (username, password) VALUES ('$_POST[user]', '$_POST[pass]')";
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
	$encPass = crypt($_POST['pass'],$passSalt);
	$sql = "SELECT * FROM users WHERE username = '$_POST[user]'";
	$result = $conn->query($sql);

	if ($result->num_rows > 0) {
    	while($row = $result->fetch_assoc()) {
			echo $row["id"];
    	}
	} else
    	echo "0";
}

if (strstr($_POST['p'],"increase")){
	$sql = "UPDATE jedi_academy SET ". $_POST['c'] ." = ". $_POST['c'] ." + 1 WHERE id = '$_POST[id]'";
	if ($conn->query($sql) === TRUE){
    	echo "successful"; 
	} else
		echo "Error: " . $sql . " " . $conn->error;
}

$conn->close();
?>