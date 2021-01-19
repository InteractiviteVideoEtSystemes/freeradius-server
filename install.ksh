#!/bin/bash
VERSION="2.10.0"
#Nom du paquetage 
PROJET=freeradius

#Creation de l'environnement de packaging rpm
function create_rpm
{
    #Cree l'environnement de creation de package
    #Creation des macros rpmbuild
    rm ~/.rpmmacros
    touch ~/.rpmmacros
    echo "%_topdir" $PWD"/rpmbuild" >> ~/.rpmmacros
    echo "%_tmppath %{_topdir}/TMP" >> ~/.rpmmacros
    echo "%_signature gpg" >> ~/.rpmmacros
    echo "%_gpg_name IVeSkey" >> ~/.rpmmacros
    echo "%_gpg_path" $PWD"/gnupg" >> ~/.rpmmacros
    echo "%vendor IVeS" >> ~/.rpmmacros
    #Import de la clef gpg IVeS
    if [[ -z $2 || $2 -ne nosign ]]
        then svn export https://svn.ives.fr/svn-libs-dev/gnupg
    fi
    mkdir -p rpmbuild
    mkdir -p rpmbuild/SOURCES
    mkdir -p rpmbuild/SPECS
    mkdir -p rpmbuild/BUILD
    mkdir -p rpmbuild/SRPMS
    mkdir -p rpmbuild/TMP
    mkdir -p rpmbuild/RPMS
    mkdir -p rpmbuild/RPMS/noarch
    mkdir -p rpmbuild/RPMS/i386
    mkdir -p rpmbuild/RPMS/x86_64
    #Recuperation de la description du package 
    cd ./rpmbuild/SPECS/
    cp ../../${PROJET}.spec ${PROJET}.spec
    cd ../SOURCES
    cp ../../redhat/freeradius-radiusd-init .
    cp ../../redhat/freeradius-logrotate .
    cp ../../redhat/freeradius-pam-conf .
    ln -s ../.. ${PROJET}

    cd ../../
    
    #Cree le package
    if [[ -z $2 || $2 -ne nosign ]]
        then rpmbuild -bb --sign $PWD/rpmbuild/SPECS/${PROJET}.spec
        else rpmbuild -bb  $PWD/rpmbuild/SPECS/${PROJET}.spec
    fi
    if [ $? == 0 ]
    then
        echo "************************* fin du rpmbuild ****************************"
        #Recuperation du rpm
        mv -f rpmbuild/RPMS/x86_64/*.rpm $PWD/.
    fi
    clean
}

function clean
{
  	# On efface les liens ainsi que le package precedemment créé
  	echo Effacement des fichiers et liens gnupg rpmbuild ${PROJET}.rpm ${TEMPDIR}/${PROJET}
  	rm -rf gnupg rpmbuild  ${TEMPDIR}/${PROJET}
}

case $1 in
  	"clean")
  		echo "Nettoyage des liens et du package crees par la cible dev"
  		clean ;;
	"prereq")
		sudo yum -y install postgresql-devel unixODBC-devel make pcre-devel MariaDB-devel zlib-devel curl-devel expat-devel openssl-devel libconfuse-devel openldap-devel libunistring-devel libpcap-devel python-devel libpcap-devel pam-devel
		sudo yum -y install net-snmp-utils net-snmp-devel ;;

  	"rpm")
		echo "Creation du rpm"
		create_rpm "$@";;
	"export")
        echo "{" >> build.properties
        echo "'VERSION': '$VERSION'," >> build.properties
        echo "'PROJET':'$PROJET'," >> build.properties
        echo "'DESTDIR':'$DESTDIR'" >> build.properties
        echo "}" >> build.properties
       ;;
  	*)
  		echo "usage: install.ksh [options]" 
  		echo "options :"
  		echo "  rpm		Generation d'un package rpm"
  		echo "  clean		Nettoie tous les fichiers cree par le present script, liens, tar.gz et rpm";;
esac
