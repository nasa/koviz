# koviz

### Koviz is a Trick simulation data plotting, visualization and analysis tool 

Installation:

```
% sudo apt install flex // or
% yum install flex
% cd ~/dev
% git clone https://github.com/nasa/koviz.git
% cd koviz
% qmake-qt5 // or "qmake PREFIX=/dev/koviz" on Ubuntu 20.04
    #If command not found, install Qt:
        #redhat> sudo yum install qt5-qtbase-devel
        #ubuntu> sudo apt install qtchooser
% make
% bin/koviz -h                 ; # for usage
% bin/koviz RUN_dir|MONTE_dir  ; # View trick run or monte carlo data

```

