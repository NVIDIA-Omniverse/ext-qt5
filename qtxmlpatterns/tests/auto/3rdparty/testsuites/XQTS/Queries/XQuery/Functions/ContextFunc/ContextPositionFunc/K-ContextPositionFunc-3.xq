(:*******************************************************:)
(: Test: K-ContextPositionFunc-3                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: fn:position() can never return 0('=').       :)
(:*******************************************************:)
empty((1, 2, 3, current-time(), current-date(), 6, 7, 8)
[position() = 0])