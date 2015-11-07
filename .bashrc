alias ls='ls --color'
alias l='ls -lha'
alias ll='ls -l'
alias la='ls -a'
alias lh='ls -h'
alias log='vim $(date +%y%m%d).log'
alias yesterday_log='vim $(date -dyesterday +%y%m%d).log'
alias grep='grep --color'
export LS_COLORS='rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=01;05;37;41:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arj=01;31:*.taz=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lz=01;31:*.xz=01;31:*.bz2=01;31:*.tbz=01;31:*.tbz2=01;31:*.bz=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.rar=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.jpg=01;35:*.jpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.axv=01;35:*.anx=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=01;36:*.au=01;36:*.flac=01;36:*.mid=01;36:*.midi=01;36:*.mka=01;36:*.mp3=01;36:*.mpc=01;36:*.ogg=01;36:*.ra=01;36:*.wav=01;36:*.axa=01;36:*.oga=01;36:*.spx=01;36:*.xspf=01;36:'
set -o emacs
bind "\C-h" 
bind -m vi-insert '\c-n':end-of-file
bind -m vi-insert '\c-e':end-of-line #bind -m vi-insert '\c-a':beginning-of-line

bind -m vi-insert '\c-d':delete-char
bind -m vi-insert '\c-l':clear-screen
export PS1
##\[\e[34;1m\]%s\[\e[0m\]
#
alias l='ls -lh'
alias ll='ls -lh'
alias go='function __go() { cd $* ; clear; l ; unset -f __go; } ;__go'
alias grm='function __grm() { rm $*; clear; l; unset -f __grm; } ;__grm'
alias s='ss -tnlp'
alias n='sudo nethogs eno16777736'
alias w='watch -n.1 -c'
alias v='nvim'
alias vi='nvim'
alias vim='nvim'





#
PS1="\[\e[33;1m\]~\[\e[0m\]\[\e[36;1m\]server\[\e[0m\]\[\e[33;1m\]~\[\e[0m\]\[\e[35;1m\]^_^\[\e[0m\]:\[\e[32;1m\]\w\[\e[0m\]\[\e[33;1m\]>\[\e[0m\] "
#
export PS1

alias showstat='watch -n.1 -c bakstat'

alias statshow='watch -n.1 "ps axu | grep unixcli | grep -v grep | grep -v watch"'
alias pack='cd ; tar cJvf server.tar.xz demotest bin src datafile java libs .bash_profile KrysProfile.sh .bashrc; cd -'

export TERM=xterm-256color
export LANG='en_US.UTF-8'



