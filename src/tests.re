let formatTypes = fun env => Util.joinList "\n" (List.map (fun (varName, type_) => "- " ^ varName ^ ": " ^ Typing_types.typeToString(type_)) env);
let formatErrors = fun errors => Util.joinList "\n" (List.map (fun error => "- " ^ error) errors);
let expect = fun label program types errors => {
  let (_actualTypes, actualErrors) = Typing.buildEnv program;

  let actualTypes = List.filter (fun (varName, _) => varName != "+" && varName != "-") _actualTypes;

  if (types == actualTypes && errors == actualErrors) {
    Js.log ("OK: " ^ label);
  } else {
    Js.log ("Error: " ^ label);

    Js.log (Ast.formatProgram program);

    if (types != actualTypes) {
      Js.log "Expected types to be:";
      Js.log (formatTypes types);
      Js.log "";
      Js.log "Actual types:";
      Js.log (formatTypes actualTypes);
    };

    if (errors != actualErrors) {
      Js.log "Expected errors to be:";
      Js.log (formatErrors errors);
      Js.log "";
      Js.log "Actual errors:";
      Js.log (formatErrors actualErrors);
    };

    Js.log "";
  }
};

expect "Proper literal types" Ast.([
  Statement (VarAssignment "x" (NumberLiteral 10)),
  Statement (VarAssignment "name" (StringLiteral "Gosha")),
]) Typing_types.([
  ("name", StringType),
  ("x", NumberType),
]) [];

expect "Proper type for number addition" Ast.([
  Statement (VarAssignment "x" (NumberLiteral 10)),
  Statement (VarAssignment "y" (Plus (VarReference "x") (NumberLiteral 1))),
]) Typing_types.([
  ("y", NumberType),
  ("x", NumberType),
]) [];

expect "Error for adding a number with a string" Ast.([
  Statement (VarAssignment "x" (NumberLiteral 10)),
  Statement (VarAssignment "y" (Plus (VarReference "x") (StringLiteral "Yo"))),
]) Typing_types.([
  ("x", NumberType),
]) [
  {|Type mismatch in '(x + "Yo")', expected a number, got a string|},
];

expect "Properly inferred types of function arguments" Ast.([
  Statement (VarAssignment "megaAdd" (SimpleFn "x" (SimpleFn "y" (Plus (VarReference "x") (VarReference "y"))))),
  Statement (VarAssignment "noOp" (SimpleFn "x" (SimpleFn "y" (Plus (NumberLiteral 1) (NumberLiteral 2))))),
]) Typing_types.([
  ("noOp", SimpleFnType AnyType (SimpleFnType AnyType NumberType)),
  ("megaAdd", SimpleFnType NumberType (SimpleFnType NumberType NumberType)),
]) [];

expect "Properly inferred types function output" Ast.([
  Statement (VarAssignment "add1" (SimpleFn "x" (Plus (VarReference "x") (NumberLiteral 1)))),
  Statement (VarAssignment "y" (FnCall (VarReference "add1") (NumberLiteral 1))),
]) Typing_types.([
  ("y", NumberType),
  ("add1", SimpleFnType NumberType NumberType),
]) [];

expect "Signal no-variable and not-a-function errors" Ast.([
  Statement (VarAssignment "x" (NumberLiteral 10)),
  Statement (VarAssignment "y1" (FnCall (VarReference "x") (NumberLiteral 1))),
  Statement (VarAssignment "y2" (VarReference "nonExistent")),
]) Typing_types.([
  ("x", NumberType),
]) [
  {|In 'x(1)', 'x' is not a function|},
  {|No variable 'nonExistent', in 'nonExistent'|},
];

expect "Types the id function properly" Ast.([
  Statement (VarAssignment "id" (SimpleFn "x" (VarReference "x"))),
  Statement (VarAssignment "five" (FnCall (VarReference "id") (NumberLiteral 5))),
  Statement (VarAssignment "idRes" (FnCall (VarReference "id") (VarReference "id"))),
]) Typing_types.([
  ("idRes", (GenericType ["A"] (SimpleFnType (GenericLabel "A") (GenericLabel "A")))),
  ("five", NumberType),
  ("id", (GenericType ["A"] (SimpleFnType (GenericLabel "A") (GenericLabel "A")))),
]) [];

expect "Infers generics properly" Ast.([
  Statement (VarAssignment "id" (SimpleFn "x" (VarReference "x"))),
  Statement (VarAssignment "delayedId" (SimpleFn "x" (SimpleFn "y" (VarReference "y")))),
  Statement (VarAssignment "const" (SimpleFn "x" (SimpleFn "y" (FnCall (VarReference "id") (VarReference "x"))))),
  Statement (VarAssignment "constFive" (FnCall (VarReference "id") (FnCall (VarReference "const") (NumberLiteral 5)))),
]) Typing_types.([
  ("constFive", (SimpleFnType AnyType NumberType)),
  ("const", (GenericType ["A"] (SimpleFnType (GenericLabel "A") (SimpleFnType AnyType (GenericLabel "A"))))),
  ("delayedId", (SimpleFnType AnyType (GenericType ["A"] (SimpleFnType (GenericLabel "A") (GenericLabel "A"))))),
  ("id", (GenericType ["A"] (SimpleFnType (GenericLabel "A") (GenericLabel "A")))),
]) [];
