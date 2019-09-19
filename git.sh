git init
git add .
git commit 

git remote -v
git remote add origin https://github.com/db00/sdl2apk.git
git push -u origin master

git config receive.denyCurrentBranch ignore
git reset --hard

#push an existing repository
git remote add rpi pi@rpi:~/sdl2apk
git push rpi master
git push -u rpi master
git remote -v
git pull rpi

git remote rm rpi

git config --global push.default simple
git config --global user.name "db0"
git config --global user.email db0@qq.com
git push origin master
git push --set-upstream origin master

ssh-keygen -t rsa -C "db0@qq.com"
#cp ~/.ssh/id_rsa.pub to github(Settings/SSH Keys)  server .ssh/authorized_kesys
ssh-copy-id -i ~/.ssh/id_rsa.pub pi@rpi
cat ~/.ssh/id_rsa.pub | ssh pi@rpi "cat - >> ~/.ssh/authorized_keys"
#ssh pi@rpi "echo '`cat ~/.ssh/id_rsa.pub`' >> ~/.ssh/authorized_keys"

ssh -T git@github.com
