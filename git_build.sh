#!/bin/bash

GIT=/app/git/1.7.12.4/LMWP3/bin/git

if [ "$2" == "" ]; then
	echo usage: $0 \<Module\> \<Branch\> \<Workspace\>
    	exit -1
else
	versionProperties=install/version.properties
	theDate=\#$(date +"%c")
	module=$1
	branch=$2
	workspace=$3
	pkgDir=$workspace/packages
	rpmFileLocation=$workspace/maven/distributor-merger-rpm/target/rpm/ERICdistributor-merger-CXC1734794/RPMS/x86_64
	pkgReleaseArea=/proj/jkadm100/eniq_events_releases
	
fi

function getProductNumber {
        product=`cat $workspace/build.cfg | grep $module | grep $branch | awk -F " " '{print $3}'`
}


function setRstate {
        revision=`cat $workspace/build.cfg | grep $module | grep $branch | awk -F " " '{print $4}'`

        if $GIT tag | grep $product-$revision; then
		rstate=`$GIT tag | grep ${product}-${revision} | tail -1 | sed s/.*-// | perl -nle 'sub nxt{$_=shift;$l=length$_;sprintf"%0${l}d",++$_}print $1.nxt($2) if/^(.*?)(\d+$)/';`
        else
                ammendment_level=01
		rstate=$revision$ammendment_level
	fi
}


function cleanup {
        if [ -d $pkgDir ] ; then
          echo "removing $pkgDir"
          rm -rf $pkgDir
        fi
}

function createTar {

    echo "Copying $rpmFile into $pkgDir"
    cp $rpmFile $pkgDir
    cd $workspace
    tar -czvf $workspace/$pkgName packages/
    echo "Copying tar file into $pkgReleaseArea"
    cp $workspace/$pkgName $pkgReleaseArea
}


function runMaven {
    mvn -f $workspace/maven/pom.xml -Droot.path=$workspace clean install 
    rsp=$?
    if [ $rsp == 1 ]; then
       exit
    fi
    mvn -f $workspace/maven/distributor-merger-rpm/pom.xml -Droot.path=$workspace clean nar:nar-unpack package -Drstate=$rstate
    rsp=$?
}

cleanup
getProductNumber
setRstate
pkgName="probe_distributor_merger_${rstate}.tar.gz"
$GIT clean -df
$GIT checkout $branch
$GIT pull

runMaven

mkdir $pkgDir


if [ $rsp == 0 ]; then
  $GIT tag $product-$rstate
  $GIT pull
  $GIT push --tag origin $branch

  rpm=`ls $rpmFileLocation`
  echo "RPM built:$rpm"
  rpmFile=$rpmFileLocation/$rpm
  echo "Creating tar file..."
  createTar
  touch $workspace/rstate.txt
  echo $rstate >> $workspace/rstate.txt

fi  

exit $rsp
