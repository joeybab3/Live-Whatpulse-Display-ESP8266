<?php
  header('Content-Type: application/json');
  $username = htmlspecialchars($_GET['username']);
  $json   = file_get_contents("http://api.whatpulse.org/user.php?user=".$username."&formatted=yes&format=json");
  $result = json_decode($json);
  if(!empty($result->error))
  {
    echo "Something went wrong: ".$result->error;
  }
  else
  {
	  $array["username"] = $result->AccountName;
	  $array["id"] = $result->UserID;
	  $array["clicks"] = $result->Clicks;
	  $array["keys"] = $result->Keys;
	  $array["upload"] = $result->Upload;
	  $array["download"] = $result->Download;
	  echo json_encode($array);
  }
  if(isset($_GET['debug']))
  {
	  echo "\n\n\n";
	var_dump($result);
  }
?>