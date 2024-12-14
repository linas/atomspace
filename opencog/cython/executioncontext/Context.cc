#include "Context.h"

namespace opencog {

   thread_local std::deque<AtomSpacePtr> current = std::deque<AtomSpacePtr>();

   AtomSpacePtr get_context_atomspace(){
printf("duuude geth cont\n");
       if (current.empty())
           return nullptr;
       return current.back();
   }
   void push_context_atomspace(AtomSpacePtr asp){
       current.push_back(asp);
printf("duuude push cont\n");
   }

   AtomSpacePtr pop_context_atomspace(){
       AtomSpacePtr result = get_context_atomspace();
printf("duuude pop cont\n");
       current.pop_back();
       return result;
   }

   void clear_context(){
printf("duuude clear cont\n");
       current.clear();
   }

} // namespace
