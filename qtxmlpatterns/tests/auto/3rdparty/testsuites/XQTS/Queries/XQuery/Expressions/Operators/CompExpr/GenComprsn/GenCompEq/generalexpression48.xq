
(:*******************************************************:)
(:Written By: Carmelo Montanez (Automatic Generation)    :)
(:Date: June 2, 2005                                :)
(:Purpose: Test of a General Expression      :)
(:with the operands set as follows          :)
(:  operand1 = Element Constructor:)
(:  operator = =:)
(:  operand2 = Sequence of multiple element nodes (single source):)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<a>10000</a> = ($input-context1/works/employee[1]/hours[1],$input-context1/works/employee[6]/hours[1])
