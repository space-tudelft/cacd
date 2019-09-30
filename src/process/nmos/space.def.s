# process : nmos
# author  : A.J. van Genderen, TU-Delft
# date    : Feb. 15, 1988

maxkeys 12

conductors :

    resM : nm       : nm : 0.04   # the exact values for
    resP : np       : np : 25     # sheet-resistivity
    resD : nd !np                 # are not known
         | nd np nb
         | nd np nx : nd : 35

transistors :

    nenh : np nd !ni !nb !nx : np nd
    ndep : np nd ni !nb !nx  : np nd

contacts :

    normcp : nm nc np     : nm np : 0
    normcd : nm nc nd !np : nm nd : 0
    burrc  : nd nb np     : nd np : 0

capacitances :

  # capacitances to ground

    capM  : nm !np !nd      :  nm : 0.03e-3
    capMe : !nm -nm !np !nd : -nm : 0.06e-9

    capP  : np !nd      :  np : 0.05e-3
    capPe : !np -np !nd : -np : 0.06e-9

    capD  : nd !np
          | nd np nb
          | nd np nx       : nd : 0.06e-3
    capDe : !nd -nd !-np
          | !nd -nd -np -nb
          | !nd -nd -np -nx : -nd : 0.34e-9

  # coupling capacitances

    capMP  : nm np      :  nm  np : 0.07e-3
    capMeP : !nm -nm np : -nm  np : 0.07e-9
    capMPe : nm !np -np :  nm -np : 0.07e-9

    capMD  : nm nd !np       :  nm  nd : 0.07e-3
    capMeD : !nm -nm nd !np  : -nm  nd : 0.07e-9
    capMDe : nm !nd -nd !-np :  nm -nd : 0.07e-9

    capPD  : np nd nx       :  np  nd : 0.07e-3
    capPeD : !np -np nd -nx : -np  nd : 0.06e-9
    capPDe : np !nd -nd nx  :  np -nd : 0.06e-9

#EOF
