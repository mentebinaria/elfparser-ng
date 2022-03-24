import "hash" // get module for get hash malwares compare

rule Kaiten : Malware
{
	meta:
	   description = "Binary analysis for possible kaiten family malware"

	strings:
		$name1 = "kaiten.c" nocase wide ascii
		$name2 = "kaiten2.c" nocase wide ascii


    condition:
		$name1 or $name2 or
		// compare hashs compare hash with potential malware kaiten
		hash.md5(0,filesize) == "ee07542f7dba6a60342424faf92af201" or // md5
		hash.sha1(0,filesize) == "966f44a6f36f80f2807fcd7f461aa3d52e77bc81" or // sha1
		hash.sha256(0, filesize) == "305901aa920493695729132cfd20cbddc9db2cf861071450a646c6a07b4a50f3"  // sha256

}
