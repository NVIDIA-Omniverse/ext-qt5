(:*******************************************************:)
(:Test: op-add-dayTimeDurations-11                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-dayTimeDurations" function used  :)
(:together with a "div" expression.                      :)
(:*******************************************************:)

(xs:dayTimeDuration("P42DT10H10M") + xs:dayTimeDuration("P10DT10H10M")) div (xs:dayTimeDuration("P42DT10H10M") + xs:dayTimeDuration("P10DT10H10M"))