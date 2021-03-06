type expression =
  | VarReference string
  | NumberLiteral int
  | StringLiteral string
  | Plus expression expression
  | Minus expression expression
  | SimpleFn string expression
  | FnCall expression expression
  ;

type statement =
  | VarAssignment string expression;

type programElement = Expression expression | Statement statement;

type program = list programElement;

let rec formatExpression = fun expr => switch expr {
  | VarReference varName => varName;
  | NumberLiteral x => string_of_int x;
  | StringLiteral x => "\"" ^ x ^ "\"";
  | Plus a b => "(" ^ (formatExpression a) ^ " + " ^ (formatExpression b) ^ ")";
  | Minus a b => "(" ^ (formatExpression a) ^ " - " ^ (formatExpression b) ^ ")";
  | SimpleFn arg1 expr => arg1 ^ " => " ^ (formatExpression expr)
  | FnCall fn arg1 => (formatExpression fn) ^ "(" ^ (formatExpression arg1) ^ ")";
};

let formatStatement = fun stmt => switch stmt {
  | VarAssignment varName expr => "var " ^ varName ^ " = " ^ (formatExpression expr);
};
let formatElement = fun element => switch element {
  | Expression e => formatExpression e;
  | Statement s => formatStatement s;
};
let formatProgram = fun program => Util.joinList "\n" (List.map formatElement program);

let isVar = fun expr varName => switch expr {
  | VarReference vn => varName == vn;
  | _ => false;
};
let isFnCall = fun expr => switch expr {
  | FnCall _ _ => true
  | _ => false
};

let rec transformPlusMinus = fun expr => switch expr {
  | Plus a b => FnCall (FnCall (VarReference "+") (transformPlusMinus a)) (transformPlusMinus b)
  | Minus a b => FnCall (FnCall (VarReference "-") (transformPlusMinus a)) (transformPlusMinus b)
  | SimpleFn argName bodyExpr => SimpleFn argName (transformPlusMinus bodyExpr)
  | FnCall fnExpr argExpr => FnCall (transformPlusMinus fnExpr) (transformPlusMinus argExpr)
  | x => x
};

