// import "hash" // get module for get hash malwares compare
// import "elf" // parser elf

rule Family_Kaiten
{
    meta:
        author = "VitorMob"
        description = "Kaiten Malware / Bot-Net "
        malpedia_version = "1.0"
        malpedia_license = "CC BY-NC-SA 4.0"
        malpedia_sharing = "TLP:WHITE"

	strings:
		$s0 = "/etc/rc.d/rc.local"
		$s1 = "/etc/rc.conf"
		$s2 = "hackzilla" nocase wide ascii // requests in Mozilla http 1.0
		$s3 = "IpwndSamsung" nocase wide ascii // create new process, cmdline(IpwndSamsung)


	condition:
        $s0 and $s1 or $s2 or $s3
}
