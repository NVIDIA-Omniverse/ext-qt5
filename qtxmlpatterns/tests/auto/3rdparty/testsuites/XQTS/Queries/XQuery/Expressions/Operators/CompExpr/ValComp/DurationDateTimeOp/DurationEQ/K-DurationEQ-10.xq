(:*******************************************************:)
(: Test: K-DurationEQ-10                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The xs:duration values -P1Y3M4DT08H4M33S and P1Y3M4DT08H4M33S are not equal. :)
(:*******************************************************:)
xs:duration("-P1Y3M4DT08H4M33S") ne xs:duration("P1Y3M4DT08H4M33S")