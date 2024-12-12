#!/usr/bin/env bash

repo_dir=$HOME/dev/koviz
distro=$(grep VERSION_ID /etc/os-release | sed 's/VERSION_ID="\(.*\)"/\1/' | cut -d. -f1)

printf "\nBuilding koviz RPM...\n\n"
printf "repo_dir    = $repo_dir\n" 
printf "distro      = $distro\n"

cd $repo_dir || exit

long_git_tag=$(git --git-dir $repo_dir/.git describe --long --tags)
printf "long tag    = $long_git_tag\n"

git_tag=$(git --git-dir $repo_dir/.git describe --long --tags | grep -E -o '^[^-]+')
printf "git tag     = $git_tag\n"

release="$(git --git-dir $repo_dir/.git describe --long --tags | cut -f2 -d "-")"
printf "git release = $release\n"

printf "\nBuild koviz\n"
#make distclean
#qmake-qt5 
#make

printf "\n\nCopy koviz executable to $HOME/rpmbuild/SOURCES...\n\n"
cp $repo_dir/bin/koviz $HOME/rpmbuild/BUILD/

# build rpm with spec file
printf "\nBuilding RPM...\n\n"

rpmbuild -bb --define "_prefix /usr/bin" \
             --define "_version $git_tag" \
             --define "_release ${release}" \
             "$repo_dir/rpm/koviz.spec"

rm $HOME/rpmbuild/BUILD/koviz
ls $HOME/rpmbuild/RPMS/x86_64/koviz*
printf "\nComplete!\n\n"
