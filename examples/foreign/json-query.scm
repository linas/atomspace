;
; json-query.scm -- json style queries.
;
; This doesn't work yet; its just some notes.
;
; We have several kinds of styles to emulate.
; One is GraphQL which uses the Schema Defintion Language SDL
; Another is grakn.ai aka vaticle aka TypeDB
;

; GraphQL /SDL examples...

type Query  {
   field_foo: Int
   bar_field: String
	numby: Float
	trufy: Boolean
	uuid: ID
	listicle(param1:String, param2: Float, thingy:Boolean):String
}

type Mutation {

}
