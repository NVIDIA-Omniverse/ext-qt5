(:*******************************************************:)
(: Test: K-SeqInsertBeforeFunc-21                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Apply a predicate to the result of fn:insert-before(). :)
(:*******************************************************:)
empty(insert-before((1, current-time(), 3), 0, (4, 5, 6))[last() - 10])