#!/bin/zsh 

LIMIT=5000
SENDERS=5;
TIMET=0.0;
TIMEN=0.0;
RECIEVERS=100;

while [ $RECIEVERS -le $LIMIT ]; do
  (&>TIMETf time ./bin/IMRC -S $SENDERS -R $RECIEVERS >> /dev/null);
  (&>TIMENf time ./bin/IMRC -S $SENDERS -R $RECIEVERS -T -1 >> /dev/null);
  TIMET=$(awk '{print $0}' TIMETf);
  TIMEN=$(awk '{print $0}' TIMENf);
  echo "$RECIEVERS	$TIMET	$TIMEN" >> plot_sconst;
  let RECIEVERS=RECIEVERS+100;
  echo "Run -S $SENDERS -R $RECIEVERS.";
done

echo "Done with constant amount of recievers, starting both dynamic."

RECIEVERS=100;
SENDERS=100;

while [ $RECIEVERS -le $LIMIT ]; do
  (&>TIMETf time ./bin/IMRC -S $SENDERS -R $RECIEVERS >> /dev/null);
  (&>TIMENf time ./bin/IMRC -S $SENDERS -R $RECIEVERS -T -1 >> /dev/null);
  TIMET=$(awk '{print $0}' TIMETf);
  TIMEN=$(awk '{print $0}' TIMENf);
  echo "$RECIEVERS	$TIMET	$TIMEN" >> plot_sdyn;
  let RECIEVERS=RECIEVERS+100;
  let SENDERS=SENDERS+100;
  echo "Run -S $SENDERS -R $RECIEVERS.";
done

echo "TESTS FINISHED, PLOTTING."

gnuplot plots_all.gp
