(:*******************************************************:)
(:Test: op-subtract-times-10                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "subtract-times" function used  :)
(:together with an "or" expression.                      :)
(: Uses the "fn:string" function to account for new EBV rules. :)
(:*******************************************************:)
 
fn:string((xs:time("13:00:00Z") - xs:time("17:00:00Z"))) or fn:string((xs:time("13:00:00Z") - xs:time("17:00:00Z")))