#!/bin/sh

chown root rt_snd
chgrp users rt_snd
chmod 4775 rt_snd

chown root clock_set
chgrp users clock_set
chmod 4775 clock_set

chown root clock_snd_skew
chgrp users clock_snd_skew
chmod 4775 clock_snd_skew

chown root rt_mtr
chgrp users rt_mtr
chmod 4775 rt_mtr

chown root rt_rcv
chgrp users rt_rcv
chmod 4775 rt_rcv
