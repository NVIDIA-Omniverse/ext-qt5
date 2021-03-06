(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than2args-9          :)
(:Written By: Carmelo Montanez                            :)
(:Date: Tue Apr 12 16:29:06 GMT-05:00 2005                :)
(:Purpose: Evaluates The "op:yearMonthDuration-greater-than" operator:)
(: with the arguments set as follows:                    :)
(:$arg1 = xs:yearMonthDuration(lower bound)             :)
(:$arg2 = xs:yearMonthDuration(mid range)               :)
(:*******************************************************:)

xs:yearMonthDuration("P0Y0M") le xs:yearMonthDuration("P1000Y6M")