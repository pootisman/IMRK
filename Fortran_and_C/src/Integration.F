! Simple IO
! Number comparison
      REAL      ::      E = 500,STP = 0.00001,RES = 0,X = 0
      INTEGER   ::      STPS, CNT
      RES = 0
      STPS = (E-X)/STP
      DO CNT=0,STPS,1
        RES = RES + ((X*X) + (X + STP)*(X + STP))*STP/2
        X = X + STP
      END DO
      END
