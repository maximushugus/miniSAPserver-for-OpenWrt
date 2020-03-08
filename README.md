# miniSAPserver-for-OpenWrt
Les sources officielles de miniSAPserver ne peuvent pas être compilées pour OpenWrt, à cause d'une variable dont le type ne semble pas être supporté sur ce système.

J'ai donc modifié les sources pour qu'elles puissent être compilées pour OpenWrt : elles sont disponibles dans le dossier "sources" de ce repo, et peuvent être compilées.

J'ai mis le programme compilé pour les processeurs MIPS dans le dossier "Binary for MIPS". Ce fichier doit être placé dans le dossier "/usr/sbin".
J'ai également fait un fichier à placer dans le dossier "/etc/init.d/" pour le lancement au démarrage du service.
Enfin le fichier de configuration (sap.cfg) doit être placé dans "/etc/config/"
