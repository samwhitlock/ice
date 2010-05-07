ICE Performance Contest
=======================
Michael Forney <mforney@berkeley.edu>
Sam Whitlock <phynominal@gmail.com>

Running
-------
Run make, then ./ice start.pbm end.pbm

Notes
-----
Our solution will usually answer quickly (within 10 seconds), but every once
in a while will take much longer. We were unable to solve this problem by the
deadline, but we think it's because it is getting falsely mislead down
incorrect paths by our calculate_score function.

We ended up using a hack to get the threads to consistantly terminate by just
calling exit. It seems that cancellation points are not created with Macs in
the necessary places, and we figured consistancy was better than quality in
this case.

Our tests no longer pass because of changes we made to the core of our program
and the changes necessary to make this consistant.

