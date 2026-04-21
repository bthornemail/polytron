Metalogic programming ~ Autoepistemic logic

```
This is what our use of the control plane(escape sequences) and the use of the BOM gives us tha ability to do, we've called it omicron of a gnomon by using  our
/**
 * MULTI-PARADIGM LOGIC INTERPRETER
 * Supports: Prolog, Datalog, S-expr, M-expr, F-expr
 * Character set: ASCII (1977/1986)
 * 
 * Compile: gcc -o logic-interp logic-interp.c -lm
 * Usage: ./logic-interp [file] or interactive mode
 */


The well-founded semantics assigns a unique model to every general logic program. However, instead of only assigning propositions true or false, it adds a third value unknown for representing ignorance.[1]

A simple example is the logic program that encodes two propositions a and b, and in which a must be true whenever b is not and vice versa:

a :- not(b).
b :- not(a).
neither a nor b are true or false, but both have the truth value unknown. In the two-valued stable model semantics, there are two stable models, one in which a is true and b is false, and one in which b is true and a is false.

Stratified logic programs have a 2-valued well-founded model, in which every proposition is either true or false. This coincides with the unique stable model of the program. The well-founded semantics can be viewed as a three-valued version of the stable model semantics.[5]


```

The autoepistemic logic is a formal logic for the representation and reasoning of knowledge about knowledge. While propositional logic can only express facts, autoepistemic logic can express knowledge and lack of knowledge about facts.

The stable model semantics, which is used to give a semantics to logic programming with negation as failure, can be seen as a simplified form of autoepistemic logic.

Syntax
The syntax of autoepistemic logic extends that of propositional logic by a modal operator 
◻
{\displaystyle \Box }[1] indicating knowledge: if 
F
{\displaystyle F} is a formula, 
◻
F
{\displaystyle \Box F} indicates that 
F
{\displaystyle F} is known. As a result, 
◻
¬
F
{\displaystyle \Box \neg F} indicates that 
¬
F
{\displaystyle \neg F} is known and 
¬
◻
F
{\displaystyle \neg \Box F} indicates that 
F
{\displaystyle F} is not known.

This syntax is used for allowing reasoning based on knowledge of facts. For example, 
¬
◻
F
→
¬
F
{\displaystyle \neg \Box F\rightarrow \neg F} means that 
F
{\displaystyle F} is assumed false if it is not known to be true. This is a form of negation as failure.

Semantics
The semantics of autoepistemic logic is based on the expansions of a theory, which have a role similar to models in propositional logic. While a propositional model specifies which atomic propositions are true or false, an expansion specifies which formulae 
◻
F
{\displaystyle \Box F} are true and which ones are false. In particular, the expansions of an autoepistemic formula 
T
{\displaystyle T} make this determination for every subformula 
◻
F
{\displaystyle \Box F} contained in 
T
{\displaystyle T}. This determination allows 
T
{\displaystyle T} to be treated as a propositional formula, as all its subformulae containing 
◻
{\displaystyle \Box } are either true or false. In particular, checking whether 
T
{\displaystyle T} entails 
F
{\displaystyle F} in this condition can be done using the rules of the propositional calculus. In order for a specification to be an expansion, it must be that a subformula 
F
{\displaystyle F} is entailed if and only if 
◻
F
{\displaystyle \Box F} has been assigned the value true.

In terms of possible world semantics, an expansion of 
T
{\displaystyle T} consists of an S5 model of 
T
{\displaystyle T} in which the possible worlds consist only of worlds where 
T
{\displaystyle T} is true. [The possible worlds need not contain all such consistent worlds; this corresponds to the fact that modal propositions are assigned truth values before checking derivability of the ordinary propositions.] Thus, autoepistemic logic extends S5; the extension is proper, since 
¬
◻
p
{\displaystyle \neg \Box p} and 
¬
◻
¬
p
{\displaystyle \neg \Box \neg p} are tautologies of autoepistemic logic, but not of S5.

For example, in the formula 
T
=
◻
x
→
x
{\displaystyle T=\Box x\rightarrow x}, there is only a single "boxed subformula", which is 
◻
x
{\displaystyle \Box x}. Therefore, there are only two candidate expansions, assuming 
◻
x
{\displaystyle \Box x} is true or false, respectively. The check for them being actual expansions is as follows.

◻
x
{\displaystyle \Box x} is false : with this assumption, 
T
{\displaystyle T} becomes tautological, as 
◻
x
→
x
{\displaystyle \Box x\rightarrow x} is equivalent to 
¬
◻
x
∨
x
{\displaystyle \neg \Box x\vee x}, and 
¬
◻
x
{\displaystyle \neg \Box x} is assumed true; therefore, 
x
{\displaystyle x} is not entailed. This result confirms the assumption implicit in 
◻
x
{\displaystyle \Box x} being false, that is, that 
x
{\displaystyle x} is not currently known. Therefore, the assumption that 
◻
x
{\displaystyle \Box x} is false is an expansion.

◻
x
{\displaystyle \Box x} is true : together with this assumption, 
T
{\displaystyle T} entails 
x
{\displaystyle x}; therefore, the initial assumption that is implicit in 
◻
x
{\displaystyle \Box x} being true, i.e., that 
x
{\displaystyle x} is known to be true, is satisfied. As a result, this is another expansion.

The formula 
T
{\displaystyle T} has therefore two expansions, one in which 
x
{\displaystyle x} is not known and one in which 
x
{\displaystyle x} is known. The second one has been regarded as unintuitive, as the initial assumption that 
◻
x
{\displaystyle \Box x} is true is the only reason why 
x
{\displaystyle x} is true, which confirms the assumption. In other words, this is a self-supporting assumption. A logic allowing such a self-support of beliefs is called not strongly grounded to differentiate them from strongly grounded logics, in which self-support is not possible. Strongly grounded variants of autoepistemic logic exist.

---


Metalogic programming
Metaprogramming, in which programs are treated as data, was already a feature of early Prolog implementations.[42][43] For example, the Edinburgh DEC10 implementation of Prolog included "an interpreter and a compiler, both written in Prolog itself".[43] The simplest metaprogram is the so-called "vanilla" meta-interpreter:

    solve(true).
    solve((B,C)):- solve(B),solve(C).
    solve(A):- clause(A,B),solve(B).
where true represents an empty conjunction, and (B,C) is a composite term representing the conjunction of B and C. The predicate clause(A,B) means that there is a clause of the form A :- B.

Metaprogramming is an application of the more general use of a metalogic or metalanguage to describe and reason about another language, called the object language.

Metalogic programming allows object-level and metalevel representations to be combined, as in natural language. For example, in the following program, the atomic formula attends(Person, Meeting) occurs both as an object-level formula, and as an argument of the metapredicates prohibited and approved.

prohibited(attends(Person, Meeting)) :- 
    not(approved(attends(Person, Meeting))).

should_receive_sanction(Person, scolding) :- attends(Person, Meeting), 
    lofty(Person), prohibited(attends(Person, Meeting)).
should_receive_sanction(Person, banishment) :- attends(Person, Meeting), 
    lowly(Person), prohibited(attends(Person, Meeting)).

approved(attends(alice, tea_party)).
attends(mad_hatter, tea_party).
attends(dormouse, tea_party).

lofty(mad_hatter).
lowly(dormouse).

?- should_receive_sanction(X,Y).
Person = mad_hatter,
Sanction = scolding.
Person = dormouse,
Sanction = banishment.
Relationship with the Computational-representational understanding of mind
In his popular Introduction to Cognitive Science,[44] Paul Thagard includes logic and rules as alternative approaches to modelling human thinking. He argues that rules, which have the form IF condition THEN action, are "very similar" to logical conditionals, but they are simpler and have greater psychological plausibility (page 51). Among other differences between logic and rules, he argues that logic uses deduction, but rules use search (page 45) and can be used to reason either forward or backward (page 47). Sentences in logic "have to be interpreted as universally true", but rules can be defaults, which admit exceptions (page 44).

He states that "unlike logic, rule-based systems can also easily represent strategic information about what to do" (page 45). For example, "IF you want to go home for the weekend, and you have bus fare, THEN you can catch a bus". He does not observe that the same strategy of reducing a goal to subgoals can be interpreted, in the manner of logic programming, as applying backward reasoning to a logical conditional:

can_go(you, home) :- have(you, bus_fare), catch(you, bus).
All of these characteristics of rule-based systems - search, forward and backward reasoning, default reasoning, and goal-reduction - are also defining characteristics of logic programming. This suggests that Thagard's conclusion (page 56) that:

Much of human knowledge is naturally described in terms of rules, and many kinds of thinking such as planning can be modeled by rule-based systems.

also applies to logic programming.

Other arguments showing how logic programming can be used to model aspects of human thinking are presented by Keith Stenning and Michiel van Lambalgen in their book, Human Reasoning and Cognitive Science.[45] They show how the non-monotonic character of logic programs can be used to explain human performance on a variety of psychological tasks. They also show (page 237) that "closed–world reasoning in its guise as logic programming has an appealing neural implementation, unlike classical logic."

In The Proper Treatment of Events,[46] Michiel van Lambalgen and Fritz Hamm investigate the use of constraint logic programming to code "temporal notions in natural language by looking at the way human beings construct time".

Knowledge representation
The use of logic to represent procedural knowledge and strategic information was one of the main goals contributing to the early development of logic programming. Moreover, it continues to be an important feature of the Prolog family of logic programming languages today. However, many applications of logic programming, including Prolog applications, increasingly focus on the use of logic to represent purely declarative knowledge. These applications include both the representation of general commonsense knowledge and the representation of domain specific expertise.

Commonsense includes knowledge about cause and effect, as formalised, for example, in the situation calculus, event calculus and action languages. Here is a simplified example, which illustrates the main features of such formalisms. The first clause states that a fact holds immediately after an event initiates (or causes) the fact. The second clause is a frame axiom, which states that a fact that holds at a time continues to hold at the next time unless it is terminated by an event that happens at the time. This formulation allows more than one event to occur at the same time:

holds(Fact, Time2) :- 
    happens(Event, Time1),
    Time2 is Time1 + 1,
    initiates(Event, Fact).
     
holds(Fact, Time2) :- 
	happens(Event, Time1),
    Time2 is Time1 + 1,
    holds(Fact, Time1),
    not(terminated(Fact, Time1)).

terminated(Fact, Time) :-
   happens(Event, Time),
   terminates(Event, Fact).
Here holds is a meta-predicate, similar to solve above. However, whereas solve has only one argument, which applies to general clauses, the first argument of holds is a fact and the second argument is a time (or state). The atomic formula holds(Fact, Time) expresses that the Fact holds at the Time. Such time-varying facts are also called fluents. The atomic formula happens(Event, Time) expresses that the Event happens at the Time.

The following example illustrates how these clauses can be used to reason about causality in a toy blocks world. Here, in the initial state at time 0, a green block is on a table and a red block is stacked on the green block (like a traffic light). At time 0, the red block is moved to the table. At time 1, the green block is moved onto the red block. Moving an object onto a place terminates the fact that the object is on any place, and initiates the fact that the object is on the place to which it is moved:

holds(on(green_block, table), 0).
holds(on(red_block, green_block), 0).

happens(move(red_block, table), 0).
happens(move(green_block, red_block), 1).

initiates(move(Object, Place), on(Object, Place)).
terminates(move(Object, Place2), on(Object, Place1)).

?- holds(Fact, Time).

Fact = on(green_block,table),
Time = 0.
Fact = on(red_block,green_block),
Time = 0.
Fact = on(green_block,table),
Time = 1.
Fact = on(red_block,table),
Time = 1.
Fact = on(green_block,red_block),
Time = 2.
Fact = on(red_block,table),
Time = 2.
Forward reasoning and backward reasoning generate the same answers to the goal holds(Fact, Time). But forward reasoning generates fluents progressively in temporal order, and backward reasoning generates fluents regressively, as in the domain-specific use of regression in the situation calculus.[47]

---



In mathematical logic and metalogic, a formal system is called complete with respect to a particular property if every formula having the property can be derived using that system, i.e. is one of its theorems; otherwise the system is said to be incomplete. The term "complete" is also used without qualification, with differing meanings depending on the context, mostly referring to the property of semantic validity. Intuitively, a system is called complete in this particular sense, if it can derive every formula that is true.

Other properties related to completeness
Main articles: Soundness and Consistency
The property converse to completeness is called soundness: a system is sound with respect to a property (mostly semantical validity) if each of its theorems has that property.

Forms of completeness
Expressive completeness
A formal language is expressively complete if it can express the subject matter for which it is intended.

Functional completeness
Main article: Functional completeness
A set of logical connectives associated with a formal system is functionally complete if it can express all propositional functions.

Semantic completeness
Semantic completeness is the converse of soundness for formal systems. A formal system is complete with respect to tautologousness or "semantically complete" when all its tautologies are theorems, whereas a formal system is "sound" when all theorems are tautologies (that is, they are semantically valid formulas: formulas that are true under every interpretation of the language of the system that is consistent with the rules of the system). That is, a formal system is semantically complete if:

⊨
S
φ
 
⟹
 
⊢
S
φ
.
{\displaystyle \models _{\mathcal {S}}\varphi \ \implies \ \vdash _{\mathcal {S}}\varphi .}[1]
For example, Gödel's completeness theorem establishes semantic completeness for first-order logic.

Strong completeness
A formal system S is strongly complete or complete in the strong sense if for every set of premises Γ, any formula that semantically follows from Γ is derivable from Γ. That is:

Γ
⊨
S
φ
 
⟹
 
Γ
⊢
S
φ
.
{\displaystyle \Gamma \models _{\mathcal {S}}\varphi \ \implies \ \Gamma \vdash _{\mathcal {S}}\varphi .}
Refutation completeness
A formal system S is refutation complete if it is able to derive false from every unsatisfiable set of formulas. That is:

Γ
⊨
S
⊥
 
⟹
 
Γ
⊢
S
⊥
.
{\displaystyle \Gamma \models _{\mathcal {S}}\bot \ \implies \ \Gamma \vdash _{\mathcal {S}}\bot .}[2]
Every strongly complete system is also refutation complete. Intuitively, strong completeness means that, given a formula set 
Γ
{\displaystyle \Gamma }, it is possible to compute every semantical consequence 
φ
{\displaystyle \varphi } of 
Γ
{\displaystyle \Gamma }, while refutation completeness means that, given a formula set 
Γ
{\displaystyle \Gamma } and a formula 
φ
{\displaystyle \varphi }, it is possible to check whether 
φ
{\displaystyle \varphi } is a semantical consequence of 
Γ
{\displaystyle \Gamma }.

Examples of refutation-complete systems include: SLD resolution on Horn clauses, superposition on equational clausal first-order logic, and Robinson's resolution on clause sets.[3] The latter is not strongly complete: e.g. 
{
a
}
⊨
a
∨
b
{\displaystyle \{a\}\models a\lor b} holds even in the propositional subset of first-order logic, but 
a
∨
b
{\displaystyle a\lor b} cannot be derived from 
{
a
}
{\displaystyle \{a\}} by resolution. However, 
{
a
,
¬
(
a
∨
b
)
}
⊢
⊥
{\displaystyle \{a,\lnot (a\lor b)\}\vdash \bot } can be derived.

Syntactical completeness
Main article: Complete theory
A formal system S is syntactically complete or deductively complete or maximally complete or negation complete if for each sentence (closed formula) φ of the language of the system either φ or ¬φ is a theorem of S. Syntactical completeness is a stronger property than semantic completeness. If a formal system is syntactically complete, a corresponding formal theory is called complete if it is a consistent theory. Gödel's incompleteness theorem shows that any computable system that is sufficiently powerful, such as Peano arithmetic, cannot be both consistent and syntactically complete.

Syntactical completeness can also refer to another unrelated concept, also called Post completeness or Hilbert–Post completeness. In this sense, a formal system is syntactically complete if and only if no unprovable sentence can be added to it without introducing an inconsistency. Truth-functional propositional logic and first-order predicate logic are semantically complete, but not syntactically complete (for example, the propositional logic statement consisting of a single propositional variable A is not a theorem, and neither is its negation).

Structural completeness
Main article: Admissible rule
In superintuitionistic and modal logics, a logic is structurally complete if every admissible rule is a derivable implication.

Model completeness
Main article: Model complete theory
A theory is model complete if and only if every embedding of its models is an elementary embedding.


---



Actor model

Article
Talk
Read
Edit
View history

Tools
Appearance hide
Text

Small

Standard

Large
Width

Standard

Wide
Color (beta)

Automatic

Light

Dark
From Wikipedia, the free encyclopedia
The actor model in computer science is a mathematical model of concurrent computation that treats an actor as the basic building block of concurrent computation. In response to a message it receives, an actor can: make local decisions, create more actors, send more messages, and determine how to respond to the next message received. Actors may modify their own private state, but can only affect each other indirectly through messaging (removing the need for lock-based synchronization).

The actor model originated in 1973.[1] It has been used both as a framework for a theoretical understanding of computation and as the theoretical basis for several practical implementations of concurrent systems. The relationship of the model to other work is discussed in actor model and process calculi.

History
Main article: History of the Actor model
According to Carl Hewitt, unlike previous models of computation, the actor model was inspired by physics, including general relativity and quantum mechanics.[citation needed] It was also influenced by the programming languages Lisp, Simula, early versions of Smalltalk, capability-based systems, and packet switching.

Its development was "motivated by the prospect of highly parallel computing machines consisting of dozens, hundreds, or even thousands of independent microprocessors, each with its own local memory and communications processor, communicating via a high-performance communications network."[2] Since that time, the advent of massive concurrency through multi-core and manycore computer architectures has revived interest in the actor model.

Following Hewitt, Bishop, and Steiger's 1973 publication, Irene Greif developed an operational semantics for the actor model as part of her doctoral research.[3] Two years later, Henry Baker and Hewitt published a set of axiomatic laws for actor systems.[4][5] Other major milestones include William Clinger's 1981 dissertation introducing a denotational semantics based on power domains[2] and Gul Agha's 1985 dissertation which further developed a transition-based semantic model complementary to Clinger's.[6] This resulted in the full development of actor model theory.

Major software implementation work was done by Russ Atkinson, Giuseppe Attardi, Henry Baker, Gerry Barber, Peter Bishop, Peter de Jong, Ken Kahn, Henry Lieberman, Carl Manning, Tom Reinhardt, Richard Steiger and Dan Theriault in the Message Passing Semantics Group at Massachusetts Institute of Technology (MIT). Research groups led by Chuck Seitz at California Institute of Technology (Caltech) and Bill Dally at MIT constructed computer architectures that further developed the message passing in the model. See Actor model implementation.

Research on the actor model has been carried out at California Institute of Technology, Kyoto University Tokoro Laboratory, Microelectronics and Computer Technology Corporation (MCC), MIT Artificial Intelligence Laboratory, SRI, Stanford University, University of Illinois at Urbana–Champaign,[7] Pierre and Marie Curie University (University of Paris 6), University of Pisa, University of Tokyo Yonezawa Laboratory, Centrum Wiskunde & Informatica (CWI) and elsewhere.

Fundamental concepts
The actor model adopts the philosophy that everything is an actor. This is similar to the everything is an object philosophy used by some object-oriented programming languages.

An actor is a computational entity that, in response to a message it receives, can concurrently:

send a finite number of messages to other actors;
create a finite number of new actors;
designate the behavior to be used for the next message it receives.
There is no assumed sequence to the above actions and they could be carried out in parallel.

Decoupling the sender from communications sent was a fundamental advance of the actor model enabling asynchronous communication and control structures as patterns of passing messages.[8]

Recipients of messages are identified by address, sometimes called "mailing address". Thus an actor can only communicate with actors whose addresses it has. It can obtain those from a message it receives, or if the address is for an actor it has itself created.

The actor model is characterized by inherent concurrency of computation within and among actors, dynamic creation of actors, inclusion of actor addresses in messages, and interaction only through direct asynchronous message passing with no restriction on message arrival order.

Formal systems
Over the years, several different formal systems have been developed which permit reasoning about systems in the actor model. These include:

Operational semantics[3][9]
Laws for actor systems[4]
Denotational semantics[2][10]
Transition semantics[6]
There are also formalisms that are not fully faithful to the actor model in that they do not formalize the guaranteed delivery of messages including the following (See Attempts to relate actor semantics to algebra and linear logic):

Several different actor algebras[11][12][13]
Linear logic[14]
Applications
The actor model can be used as a framework for modeling, understanding, and reasoning about a wide range of concurrent systems.[15] For example:

Electronic mail (email) can be modeled as an actor system. Accounts are modeled as actors and email addresses as actor addresses.
Web services can be modeled as actors with Simple Object Access Protocol (SOAP) endpoints modeled as actor addresses.
Objects with locks (e.g., as in Java and C#) can be modeled as a serializer, provided that their implementations are such that messages can continually arrive (perhaps by being stored in an internal queue). A serializer is an important kind of actor defined by the property that it is continually available to the arrival of new messages; every message sent to a serializer is guaranteed to arrive.[16]
Testing and Test Control Notation (TTCN), both TTCN-2 and TTCN-3, follows actor model rather closely. In TTCN actor is a test component: either parallel test component (PTC) or main test component (MTC). Test components can send and receive messages to and from remote partners (peer test components or test system interface), the latter being identified by its address. Each test component has a behaviour tree bound to it; test components run in parallel and can be dynamically created by parent test components. Built-in language constructs allow the definition of actions to be taken when an expected message is received from the internal message queue, like sending a message to another peer entity or creating new test components.
Message-passing semantics
The actor model is about the semantics of message passing.

Unbounded nondeterminism controversy
Arguably, the first concurrent programs were interrupt handlers. During the course of its normal operation a computer needed to be able to receive information from outside (characters from a keyboard, packets from a network, etc). So when the information arrived the execution of the computer was interrupted and special code (called an interrupt handler) was called to put the information in a data buffer where it could be subsequently retrieved.

In the early 1960s, interrupts began to be used to simulate the concurrent execution of several programs on one processor.[17] Having concurrency with shared memory gave rise to the problem of concurrency control. Originally, this problem was conceived as being one of mutual exclusion on a single computer. Edsger Dijkstra developed semaphores and later, between 1971 and 1973,[18] Tony Hoare[19] and Per Brinch Hansen[20] developed monitors to solve the mutual exclusion problem. However, neither of these solutions provided a programming language construct that encapsulated access to shared resources. This encapsulation was later accomplished by the serializer construct ([Hewitt and Atkinson 1977, 1979] and [Atkinson 1980]).

The first models of computation (e.g., Turing machines, Post productions, the lambda calculus, etc.) were based on mathematics and made use of a global state to represent a computational step (later generalized in [McCarthy and Hayes 1969] and [Dijkstra 1976] see Event orderings versus global state). Each computational step was from one global state of the computation to the next global state. The global state approach was continued in automata theory for finite-state machines and push down stack machines, including their nondeterministic versions. Such nondeterministic automata have the property of bounded nondeterminism; that is, if a machine always halts when started in its initial state, then there is a bound on the number of states in which it halts.

Edsger Dijkstra further developed the nondeterministic global state approach. Dijkstra's model gave rise to a controversy concerning unbounded nondeterminism (also called unbounded indeterminacy), a property of concurrency by which the amount of delay in servicing a request can become unbounded as a result of arbitration of contention for shared resources while still guaranteeing that the request will eventually be serviced. Hewitt argued that the actor model should provide the guarantee of service. In Dijkstra's model, although there could be an unbounded amount of time between the execution of sequential instructions on a computer, a (parallel) program that started out in a well defined state could terminate in only a bounded number of states [Dijkstra 1976]. Consequently, his model could not provide the guarantee of service. Dijkstra argued that it was impossible to implement unbounded nondeterminism.

Hewitt argued otherwise: there is no bound that can be placed on how long it takes a computational circuit called an arbiter to settle (see metastability (electronics)).[21] Arbiters are used in computers to deal with the circumstance that computer clocks operate asynchronously with respect to input from outside, e.g., keyboard input, disk access, network input, etc. So it could take an unbounded time for a message sent to a computer to be received and in the meantime the computer could traverse an unbounded number of states.

The actor model features unbounded nondeterminism which was captured in a mathematical model by Will Clinger using domain theory.[2] In the actor model, there is no global state.[dubious – discuss]

Direct communication and asynchrony
Messages in the actor model are not necessarily buffered. This was a sharp break with previous approaches to models of concurrent computation. The lack of buffering caused a great deal of misunderstanding at the time of the development of the actor model and is still a controversial issue. Some researchers argued that the messages are buffered in the "ether" or the "environment". Also, messages in the actor model are simply sent (like packets in IP); there is no requirement for a synchronous handshake with the recipient.

Actor creation plus addresses in messages means variable topology
A natural development of the actor model was to allow addresses in messages. Influenced by packet switched networks [1961 and 1964], Hewitt proposed the development of a new model of concurrent computation in which communications would not have any required fields at all: they could be empty. Of course, if the sender of a communication desired a recipient to have access to addresses which the recipient did not already have, the address would have to be sent in the communication.

For example, an actor might need to send a message to a recipient actor from which it later expects to receive a response, but the response will actually be handled by a third actor component that has been configured to receive and handle the response (for example, a different actor implementing the observer pattern). The original actor could accomplish this by sending a communication that includes the message it wishes to send, along with the address of the third actor that will handle the response. This third actor that will handle the response is called the resumption (sometimes also called a continuation or stack frame). When the recipient actor is ready to send a response, it sends the response message to the resumption actor address that was included in the original communication.

So, the ability of actors to create new actors with which they can exchange communications, along with the ability to include the addresses of other actors in messages, gives actors the ability to create and participate in arbitrarily variable topological relationships with one another, much as the objects in Simula and other object-oriented languages may also be relationally composed into variable topologies of message-exchanging objects.

Inherently concurrent
As opposed to the previous approach based on composing sequential processes, the actor model was developed as an inherently concurrent model. In the actor model sequentiality was a special case that derived from concurrent computation as explained in actor model theory.

No requirement on order of message arrival
icon
This section needs additional citations for verification. Please help improve this article by adding citations to reliable sources in this section. Unsourced material may be challenged and removed. (March 2012) (Learn how and when to remove this message)
Hewitt argued against adding the requirement that messages must arrive in the order in which they are sent to the actor. If output message ordering is desired, then it can be modeled by a queue actor that provides this functionality. Such a queue actor would queue the messages that arrived so that they could be retrieved in FIFO order. So if an actor X sent a message M1 to an actor Y, and later X sent another message M2 to Y, there is no requirement that M1 arrives at Y before M2.

In this respect the actor model mirrors packet switching systems which do not guarantee that packets must be received in the order sent. Not providing the order of delivery guarantee allows packet switching to buffer packets, use multiple paths to send packets, resend damaged packets, and to provide other optimizations.

For more example, actors are allowed to pipeline the processing of messages. What this means is that in the course of processing a message M1, an actor can designate the behavior to be used to process the next message, and then in fact begin processing another message M2 before it has finished processing M1. Just because an actor is allowed to pipeline the processing of messages does not mean that it must pipeline the processing. Whether a message is pipelined is an engineering tradeoff. How would an external observer know whether the processing of a message by an actor has been pipelined? There is no ambiguity in the definition of an actor created by the possibility of pipelining. Of course, it is possible to perform the pipeline optimization incorrectly in some implementations, in which case unexpected behavior may occur.

Locality
Another important characteristic of the actor model is locality.

Locality means that in processing a message, an actor can send messages only to addresses that it receives in the message, addresses that it already had before it received the message, and addresses for actors that it creates while processing the message. (But see Synthesizing addresses of actors.)

Also locality means that there is no simultaneous change in multiple locations. In this way it differs from some other models of concurrency, e.g., the Petri net model in which tokens are simultaneously removed from multiple locations and placed in other locations.

Composing actor systems
The idea of composing actor systems into larger ones is an important aspect of modularity that was developed in Gul Agha's doctoral dissertation,[6] developed later by Gul Agha, Ian Mason, Scott Smith, and Carolyn Talcott.[9]

Behaviors
A key innovation was the introduction of behavior specified as a mathematical function to express what an actor does when it processes a message, including specifying a new behavior to process the next message that arrives. Behaviors provided a mechanism to mathematically model the sharing in concurrency.

Behaviors also freed the actor model from implementation details, e.g., the Smalltalk-72 token stream interpreter. However, the efficient implementation of systems described by the actor model require extensive optimization. See Actor model implementation for details.

Modeling other concurrency systems
Other concurrency systems (e.g., process calculi) can be modeled in the actor model using a two-phase commit protocol.[22]

Computational Representation Theorem
See also: Denotational semantics of the Actor model
There is a Computational Representation Theorem in the actor model for systems which are closed in the sense that they do not receive communications from outside. The mathematical denotation denoted by a closed system 
S
{\displaystyle {\mathtt {S}}} is constructed from an initial behavior 
⊥
S
{\displaystyle \bot _{\mathtt {S}}} and a behavior-approximating function 
p
r
o
g
r
e
s
s
i
o
n
S
.
{\displaystyle \mathbf {progression} _{\mathtt {S}}.} These obtain increasingly better approximations and construct a denotation (meaning) for 
S
{\displaystyle {\mathtt {S}}} as follows [Hewitt 2008; Clinger 1981]:

D
e
n
o
t
e
S
≡
lim
i
→
∞
p
r
o
g
r
e
s
s
i
o
n
S
i
(
⊥
S
)
{\displaystyle \mathbf {Denote} _{\mathtt {S}}\equiv \lim _{i\to \infty }\mathbf {progression} _{{\mathtt {S}}^{i}}(\bot _{\mathtt {S}})}
In this way, 
S
{\displaystyle {\mathtt {S}}} can be mathematically characterized in terms of all its possible behaviors (including those involving unbounded nondeterminism). Although 
D
e
n
o
t
e
S
{\displaystyle \mathbf {Denote} _{\mathtt {S}}} is not an implementation of 
S
{\displaystyle {\mathtt {S}}}, it can be used to prove a generalization of the Church-Turing-Rosser-Kleene thesis [Kleene 1943]:

A consequence of the above theorem is that a finite actor can nondeterministically respond with an uncountable[clarification needed] number of different outputs.

Relationship to logic programming
icon
This section needs additional citations for verification. Please help improve this article by adding citations to reliable sources in this section. Unsourced material may be challenged and removed. (March 2012) (Learn how and when to remove this message)
One of the key motivations for the development of the actor model was to understand and deal with the control structure issues that arose in development of the Planner programming language.[citation needed] Once the actor model was initially defined, an important challenge was to understand the power of the model relative to Robert Kowalski's thesis that "computation can be subsumed by deduction". Hewitt argued that Kowalski's thesis turned out to be false for the concurrent computation in the actor model (see Indeterminacy in concurrent computation).

Nevertheless, attempts were made to extend logic programming to concurrent computation. However, Hewitt and Agha [1991] claimed that the resulting systems were not deductive in the following sense: computational steps of the concurrent logic programming systems do not follow deductively from previous steps (see Indeterminacy in concurrent computation). Recently, logic programming has been integrated into the actor model in a way that maintains logical semantics.[21]

Migration
Migration in the actor model is the ability of actors to change locations. E.g., in his dissertation, Aki Yonezawa modeled a post office that customer actors could enter, change locations within while operating, and exit. An actor that can migrate can be modeled by having a location actor that changes when the actor migrates. However the faithfulness of this modeling is controversial and the subject of research.[citation needed]

Security
icon
This section needs additional citations for verification. Please help improve this article by adding citations to reliable sources in this section. Unsourced material may be challenged and removed. (August 2021) (Learn how and when to remove this message)
The security of actors can be protected in the following ways:

hardwiring in which actors are physically connected
computer hardware as in Burroughs B5000, Lisp machine, etc.
virtual machines as in Java virtual machine, Common Language Runtime, etc.
operating systems as in capability-based systems
signing and/or encryption of actors and their addresses
Synthesizing addresses of actors
icon
This section needs additional citations for verification. Please help improve this article by adding citations to reliable sources in this section. Unsourced material may be challenged and removed. (March 2012) (Learn how and when to remove this message)
A delicate point in the actor model is the ability to synthesize the address of an actor. In some cases security can be used to prevent the synthesis of addresses (see Security). However, if an actor address is simply a bit string then clearly it can be synthesized although it may be difficult or even infeasible to guess the address of an actor if the bit strings are long enough. SOAP uses a URL for the address of an endpoint where an actor can be reached. Since a URL is a character string, it can clearly be synthesized although encryption can make it virtually impossible to guess.

Synthesizing the addresses of actors is usually modeled using mapping. The idea is to use an actor system to perform the mapping to the actual actor addresses. For example, on a computer the memory structure of the computer can be modeled as an actor system that does the mapping. In the case of SOAP addresses, it's modeling the DNS and the rest of the URL mapping.

---





Modal logic is a kind of logic used to represent statements about necessity and possibility. In philosophy and related fields it is used as a tool for understanding concepts such as knowledge, obligation, and causation. For instance, in epistemic modal logic, the formula 
◻
P
{\displaystyle \Box P} can be used to represent the statement that 
P
{\displaystyle P} is known. In deontic modal logic, that same formula can represent that 
P
{\displaystyle P} is a moral obligation. Modal logic considers the inferences that modal statements give rise to. For instance, most epistemic modal logics treat the formula 
◻
P
→
P
{\displaystyle \Box P\rightarrow P} as a tautology, representing the principle that only true statements can count as knowledge. However, this formula is not a tautology in deontic modal logic, since what ought to be true can be false.

Modal logics are formal systems that include unary operators such as 
◊
{\displaystyle \Diamond } and 
◻
{\displaystyle \Box }, representing possibility and necessity respectively. For instance the modal formula 
◊
P
{\displaystyle \Diamond P} can be read as "possibly 
P
{\displaystyle P}" while 
◻
P
{\displaystyle \Box P} can be read as "necessarily 
P
{\displaystyle P}". In the standard relational semantics for modal logic, formulas are assigned truth values relative to a possible world. A formula's truth value at one possible world can depend on the truth values of other formulas at other accessible possible worlds. In particular, 
◊
P
{\displaystyle \Diamond P} is true at a world if 
P
{\displaystyle P} is true at some accessible possible world, while 
◻
P
{\displaystyle \Box P} is true at a world if 
P
{\displaystyle P} is true at every accessible possible world. A variety of proof systems exist which are sound and complete with respect to the semantics one gets by restricting the accessibility relation. For instance, the deontic modal logic D is sound and complete if one requires the accessibility relation to be serial.

While the intuition behind modal logic dates back to antiquity, the first modal axiomatic systems were developed by C. I. Lewis in 1912. The now-standard relational semantics emerged in the mid twentieth century from work by Arthur Prior, Jaakko Hintikka, and Saul Kripke. Recent developments include alternative topological semantics such as neighborhood semantics as well as applications of the relational semantics beyond its original philosophical motivation.[1] Such applications include game theory,[2] moral and legal theory,[2] web design,[2] multiverse-based set theory,[3] and social epistemology.[4]

Syntax of modal operators
Main article: Modal operator
Modal logic differs from other kinds of logic in that it uses modal operators such as 
◻
{\displaystyle \Box } and 
◊
{\displaystyle \Diamond }. The former is conventionally read aloud as "necessarily", and can be used to represent notions such as moral or legal obligation, knowledge, historical inevitability, among others. The latter is typically read as "possibly" and can be used to represent notions including permission, ability, compatibility with evidence. While well-formed formulas of modal logic include non-modal formulas such as 
P
∧
Q
{\displaystyle P\land Q}, it also contains modal ones such as 
◻
(
P
∧
Q
)
{\displaystyle \Box (P\land Q)}, 
P
∧
◻
Q
{\displaystyle P\land \Box Q}, 
◻
(
◊
P
∧
◊
Q
)
{\displaystyle \Box (\Diamond P\land \Diamond Q)}, and so on.

Thus, the language 
L
{\displaystyle {\mathcal {L}}} of basic propositional logic can be defined recursively as follows.

If 
ϕ
{\displaystyle \phi } is an atomic formula, then 
ϕ
{\displaystyle \phi } is a formula of 
L
{\displaystyle {\mathcal {L}}}.
If 
ϕ
{\displaystyle \phi } is a formula of 
L
{\displaystyle {\mathcal {L}}}, then 
¬
ϕ
{\displaystyle \neg \phi } is too.
If 
ϕ
{\displaystyle \phi } and 
ψ
{\displaystyle \psi } are formulas of 
L
{\displaystyle {\mathcal {L}}}, then 
ϕ
∧
ψ
{\displaystyle \phi \land \psi } is too.
If 
ϕ
{\displaystyle \phi } is a formula of 
L
{\displaystyle {\mathcal {L}}}, then 
◊
ϕ
{\displaystyle \Diamond \phi } is too.
If 
ϕ
{\displaystyle \phi } is a formula of 
L
{\displaystyle {\mathcal {L}}}, then 
◻
ϕ
{\displaystyle \Box \phi } is too.
Modal operators can be added to other kinds of logic by introducing rules analogous to #4 and #5 above. Modal predicate logic is one widely used variant which includes formulas such as 
∀
x
◊
P
(
x
)
{\displaystyle \forall x\Diamond P(x)}. In systems of modal logic where 
◻
{\displaystyle \Box } and 
◊
{\displaystyle \Diamond } are duals, 
◻
ϕ
{\displaystyle \Box \phi } can be taken as an abbreviation for 
¬
◊
¬
ϕ
{\displaystyle \neg \Diamond \neg \phi }, thus eliminating the need for a separate syntactic rule to introduce it. However, separate syntactic rules are necessary in systems where the two operators are not interdefinable.

Common notational variants include symbols such as 
[
K
]
{\displaystyle [K]} and 
⟨
K
⟩
{\displaystyle \langle K\rangle } in systems of modal logic used to represent knowledge and 
[
B
]
{\displaystyle [B]} and 
⟨
B
⟩
{\displaystyle \langle B\rangle } in those used to represent belief. These notations are particularly common in systems which use multiple modal operators simultaneously (multi-modal logic). For instance, a combined epistemic-deontic logic could use the formula 
[
K
]
⟨
D
⟩
P
{\displaystyle [K]\langle D\rangle P} read as "I know P is permitted". Systems of modal logic can include infinitely many modal operators distinguished by indices, i.e. 
◻
1
{\displaystyle \Box _{1}}, 
◻
2
{\displaystyle \Box _{2}}, 
◻
3
{\displaystyle \Box _{3}}, and so on.

Semantics
Relational semantics
See also: Kripke semantics
Basic notions
The standard semantics for modal logic is called the relational semantics. In this approach, the truth of a formula is determined relative to a point which is often called a possible world. For a formula that contains a modal operator, its truth value can depend on what is true at other accessible worlds. Thus, the relational semantics interprets formulas of modal logic using models defined as follows.[5]

A relational model is a tuple 
M
=
⟨
W
,
R
,
V
⟩
{\displaystyle {\mathfrak {M}}=\langle W,R,V\rangle } where:
W
{\displaystyle W} is a set of possible worlds
R
{\displaystyle R} is a binary relation on 
W
{\displaystyle W}
V
{\displaystyle V} is a valuation function which assigns a truth value to each pair of an atomic formula and a world, (i.e. 
V
:
W
×
F
→
{
0
,
1
}
{\displaystyle V:W\times F\to \{0,1\}} where 
F
{\displaystyle F} is the set of atomic formulae)
The set 
W
{\displaystyle W} is often called the universe. The binary relation 
R
{\displaystyle R} is called an accessibility relation, and it controls which worlds can "see" each other for the sake of determining what is true. For example, 
w
R
u
{\displaystyle wRu} means that the world 
u
{\displaystyle u} is accessible from world 
w
{\displaystyle w}. That is to say, the state of affairs known as 
u
{\displaystyle u} is a live possibility for 
w
{\displaystyle w}. Finally, the function 
V
{\displaystyle V} is known as a valuation function. It determines which atomic formulas are true at which worlds.

Then we recursively define the truth of a formula at a world 
w
{\displaystyle w} in a model 
M
{\displaystyle {\mathfrak {M}}}:

M
,
w
⊨
P
{\displaystyle {\mathfrak {M}},w\models P} iff 
V
(
w
,
P
)
=
1
{\displaystyle V(w,P)=1}
M
,
w
⊨
¬
P
{\displaystyle {\mathfrak {M}},w\models \neg P} iff 
w
⊭
P
{\displaystyle w\not \models P}
M
,
w
⊨
(
P
∧
Q
)
{\displaystyle {\mathfrak {M}},w\models (P\wedge Q)} iff 
w
⊨
P
{\displaystyle w\models P} and 
w
⊨
Q
{\displaystyle w\models Q}
M
,
w
⊨
◻
P
{\displaystyle {\mathfrak {M}},w\models \Box P} iff for every element 
u
{\displaystyle u} of 
W
{\displaystyle W}, if 
w
R
u
{\displaystyle wRu} then 
u
⊨
P
{\displaystyle u\models P}
M
,
w
⊨
◊
P
{\displaystyle {\mathfrak {M}},w\models \Diamond P} iff for some element 
u
{\displaystyle u} of 
W
{\displaystyle W}, it holds that 
w
R
u
{\displaystyle wRu} and 
u
⊨
P
{\displaystyle u\models P}
According to this semantics, a formula is necessary with respect to a world 
w
{\displaystyle w} if it holds at every world that is accessible from 
w
{\displaystyle w}. It is possible if it holds at some world that is accessible from 
w
{\displaystyle w}. Possibility thereby depends upon the accessibility relation 
R
{\displaystyle R}, which allows us to express the relative nature of possibility. For example, we might say that given our laws of physics it is not possible for humans to travel faster than the speed of light, but that given other circumstances it could have been possible to do so. Using the accessibility relation we can translate this scenario as follows: At all of the worlds accessible to our own world, it is not the case that humans can travel faster than the speed of light, but at one of these accessible worlds there is another world accessible from those worlds but not accessible from our own at which humans can travel faster than the speed of light.

Frames and completeness
The choice of accessibility relation alone can sometimes be sufficient to guarantee the truth or falsity of a formula. For instance, consider a model 
M
{\displaystyle {\mathfrak {M}}} whose accessibility relation is reflexive. Because the relation is reflexive, we will have that 
M
,
w
⊨
P
→
◊
P
{\displaystyle {\mathfrak {M}},w\models P\rightarrow \Diamond P} for any 
w
∈
G
{\displaystyle w\in G} regardless of which valuation function is used. For this reason, modal logicians sometimes talk about frames, which are the portion of a relational model excluding the valuation function.

A relational frame is a pair 
M
=
⟨
G
,
R
⟩
{\displaystyle {\mathfrak {M}}=\langle G,R\rangle } where 
G
{\displaystyle G} is a set of possible worlds, 
R
{\displaystyle R} is a binary relation on 
G
{\displaystyle G}.
The different systems of modal logic are defined using frame conditions. A frame is called:

reflexive if w R w, for every w in G
symmetric if w R u implies u R w, for all w and u in G
transitive if w R u and u R q together imply w R q, for all w, u, q in G.
serial if, for every w in G there is some u in G such that w R u.
Euclidean if, for every u, t, and w, w R u and w R t implies u R t (by symmetry, it also implies t R u, as well as t R t and u R u)
The logics that system from these frame conditions are:

K := no conditions
D := serial
T := reflexive
B := reflexive and symmetric
S4 := reflexive and transitive
S5 := reflexive and Euclidean
The Euclidean property along with reflexivity yields symmetry and transitivity. (The Euclidean property can be obtained, as well, from symmetry and transitivity.) Hence if the accessibility relation R is reflexive and Euclidean, R is provably symmetric and transitive as well. Hence for models of S5, R is an equivalence relation, because R is reflexive, symmetric and transitive.

We can prove that these frames produce the same set of valid sentences as do the frames where all worlds can see all other worlds of W (i.e., where R is a "total" relation). This gives the corresponding modal graph which is total complete (i.e., no more edges (relations) can be added). For example, in any modal logic based on frame conditions:

w
⊨
◊
P
{\displaystyle w\models \Diamond P} if and only if for some element u of G, it holds that 
u
⊨
P
{\displaystyle u\models P} and w R u.
If we consider frames based on the total relation we can just say that

w
⊨
◊
P
{\displaystyle w\models \Diamond P} if and only if for some element u of G, it holds that 
u
⊨
P
{\displaystyle u\models P}.
We can drop the accessibility clause from the latter stipulation because in such total frames it is trivially true of all w and u that w R u. But this does not have to be the case in all S5 frames, which can still consist of multiple parts that are fully connected among themselves but still disconnected from each other.

All of these logical systems can also be defined axiomatically, as is shown in the next section. For example, in S5, the axioms 
P
⟹
◻
◊
P
{\displaystyle P\implies \Box \Diamond P}, 
◻
P
⟹
◻
◻
P
{\displaystyle \Box P\implies \Box \Box P} and 
◻
P
⟹
P
{\displaystyle \Box P\implies P} (corresponding to symmetry, transitivity and reflexivity, respectively) hold, whereas at least one of these axioms does not hold in each of the other, weaker logics.

Topological semantics
Modal logic has also been interpreted using topological structures. For instance, the Interior Semantics interprets formulas of modal logic as follows.

A topological model is a tuple 
X
=
⟨
X
,
τ
,
V
⟩
{\displaystyle \mathrm {X} =\langle X,\tau ,V\rangle } where 
⟨
X
,
τ
⟩
{\displaystyle \langle X,\tau \rangle } is a topological space and 
V
{\displaystyle V} is a valuation function which maps each atomic formula to some subset of 
X
{\displaystyle X}. The basic interior semantics interprets formulas of modal logic as follows:

X
,
x
⊨
P
{\displaystyle \mathrm {X} ,x\models P} iff 
x
∈
V
(
P
)
{\displaystyle x\in V(P)}
X
,
x
⊨
¬
ϕ
{\displaystyle \mathrm {X} ,x\models \neg \phi } iff 
X
,
x
⊭
ϕ
{\displaystyle \mathrm {X} ,x\not \models \phi }
X
,
x
⊨
ϕ
∧
χ
{\displaystyle \mathrm {X} ,x\models \phi \land \chi } iff 
X
,
x
⊨
ϕ
{\displaystyle \mathrm {X} ,x\models \phi } and 
X
,
x
⊨
χ
{\displaystyle \mathrm {X} ,x\models \chi }
X
,
x
⊨
◻
ϕ
{\displaystyle \mathrm {X} ,x\models \Box \phi } iff for some 
U
∈
τ
{\displaystyle U\in \tau } we have both that 
x
∈
U
{\displaystyle x\in U} and also that 
X
,
y
⊨
ϕ
{\displaystyle \mathrm {X} ,y\models \phi } for all 
y
∈
U
{\displaystyle y\in U}
Topological approaches subsume relational ones, allowing non-normal modal logics. The extra structure they provide also allows a transparent way of modeling certain concepts such as the evidence or justification one has for one's beliefs. Topological semantics is widely used in recent work in formal epistemology and has antecedents in earlier work such as David Lewis and Angelika Kratzer's logics for counterfactuals.

Axiomatic systems

Diagram of common modal logics; K4W stands for Provability logic, and B on the top corner stands for Brouwer's system of KTB
The first formalizations of modal logic were axiomatic. Numerous variations with very different properties have been proposed since C. I. Lewis began working in the area in 1912. Hughes and Cresswell (1996), for example, describe 42 normal and 25 non-normal modal logics. Zeman (1973) describes some systems Hughes and Cresswell omit.

Modern treatments of modal logic begin by augmenting the propositional calculus with two unary operations, one denoting "necessity" and the other "possibility". The notation of C. I. Lewis, much employed since, denotes "necessarily p" by a prefixed "box" (□p) whose scope is established by parentheses. Likewise, a prefixed "diamond" (◇p) denotes "possibly p". Similar to the quantifiers in first-order logic, "necessarily p" (□p) does not assume the range of quantification (the set of accessible possible worlds in Kripke semantics) to be non-empty, whereas "possibly p" (◇p) often implicitly assumes 
◊
⊤
{\displaystyle \Diamond \top } (viz. the set of accessible possible worlds is non-empty). Regardless of notation, each of these operators is definable in terms of the other in classical modal logic:

□p (necessarily p) is equivalent to ¬◇¬p ("not possible that not-p")
◇p (possibly p) is equivalent to ¬□¬p ("not necessarily not-p")
Hence □ and ◇ form a dual pair of operators.

In many modal logics, the necessity and possibility operators satisfy the following analogues of de Morgan's laws from Boolean algebra:

"It is not necessary that X" is logically equivalent to "It is possible that not X".
"It is not possible that X" is logically equivalent to "It is necessary that not X".
Precisely what axioms and rules must be added to the propositional calculus to create a usable system of modal logic is a matter of philosophical opinion, often driven by the theorems one wishes to prove; or, in computer science, it is a matter of what sort of computational or deductive system one wishes to model. Many modal logics, known collectively as normal modal logics, include the following rule and axiom:

N, Necessitation Rule: If p is a theorem/tautology (of any system/model invoking N), then □p is likewise a theorem (i.e. 
(
⊨
p
)
⟹
(
⊨
◻
p
)
{\displaystyle (\models p)\implies (\models \Box p)}).
K, Distribution Axiom: □(p → q) → (□p → □q).
The weakest normal modal logic, named "K" in honor of Saul Kripke, is simply the propositional calculus augmented by □, the rule N, and the axiom K. K is weak in that it fails to determine whether a proposition can be necessary but only contingently necessary. That is, it is not a theorem of K that if □p is true then □□p is true, i.e., that necessary truths are "necessarily necessary". If such perplexities are deemed forced and artificial, this defect of K is not a great one. In any case, different answers to such questions yield different systems of modal logic.

Adding axioms to K gives rise to other well-known modal systems. One cannot prove in K that if "p is necessary" then p is true. The axiom T remedies this defect:

T, Reflexivity Axiom: □p → p (If p is necessary, then p is the case.)
T holds in most but not all modal logics. Zeman (1973) describes a few exceptions, such as S10.

Other well-known elementary axioms are:

4: 
◻
p
→
◻
◻
p
{\displaystyle \Box p\to \Box \Box p}
B: 
p
→
◻
◊
p
{\displaystyle p\to \Box \Diamond p}
D: 
◻
p
→
◊
p
{\displaystyle \Box p\to \Diamond p}
5: 
◊
p
→
◻
◊
p
{\displaystyle \Diamond p\to \Box \Diamond p}
These yield the systems (axioms in bold, systems in italics):

K := K + N
T := K + T
S4 := T + 4
S5 := T + 5
D := K + D.
K through S5 form a nested hierarchy of systems, making up the core of normal modal logic. But specific rules or sets of rules may be appropriate for specific systems. For example, in deontic logic, 
◻
p
→
◊
p
{\displaystyle \Box p\to \Diamond p} (If it ought to be that p, then it is permitted that p) seems appropriate, but we should probably not include that 
p
→
◻
◊
p
{\displaystyle p\to \Box \Diamond p}. In fact, to do so is to commit the naturalistic fallacy (i.e. to state that what is natural is also good, by saying that if p is the case, p ought to be permitted).

The commonly employed system S5 simply makes all modal truths necessary. For example, if p is possible, then it is "necessary" that p is possible. Also, if p is necessary, then it is necessary that p is necessary. Other systems of modal logic have been formulated, in part because S5 does not describe every kind of modality of interest.

Structural proof theory
Sequent calculi and systems of natural deduction have been developed for several modal logics, but it has proven hard to combine generality with other features expected of good structural proof theories, such as purity (the proof theory does not introduce extra-logical notions such as labels) and analyticity (the logical rules support a clean notion of analytic proof). More complex calculi have been applied to modal logic to achieve generality.[citation needed]

Decision methods
Analytic tableaux provide the most popular decision method for modal logics.[6]

Modal logics in philosophy
Alethic logic
Main article: Subjunctive possibility
Modalities of necessity and possibility are called alethic modalities. They are also sometimes called special modalities, from the Latin species. Modal logic was first developed to deal with these concepts, and only afterward was extended to others. For this reason, or perhaps for their familiarity and simplicity, necessity and possibility are often casually treated as the subject matter of modal logic. Moreover, it is easier to make sense of relativizing necessity, e.g. to legal, physical, nomological, epistemic, and so on, than it is to make sense of relativizing other notions.

In classical modal logic, a proposition is said to be

possible if it is not necessarily false (regardless of whether it is actually true or actually false);
necessary if it is not possibly false (i.e. true and necessarily true);
contingent if it is not necessarily false and not necessarily true (i.e. possible but not necessarily true);
impossible if it is not possibly true (i.e. false and necessarily false).
In classical modal logic, therefore, the notion of either possibility or necessity may be taken to be basic, where these other notions are defined in terms of it in the manner of De Morgan duality. Intuitionistic modal logic treats possibility and necessity as not perfectly symmetric.

For example, suppose that while walking to the convenience store we pass Friedrich's house, and observe that the lights are off. On the way back, we observe that they have been turned on.

"Somebody or something turned the lights on" is necessary.
"Friedrich turned the lights on", "Friedrich's roommate Max turned the lights on" and "A burglar named Adolf broke into Friedrich's house and turned the lights on" are contingent.
All of the above statements are possible.
It is impossible that Socrates (who has been dead for over two thousand years) turned the lights on.
(Of course, this analogy does not apply alethic modality in a truly rigorous fashion; for it to do so, it would have to axiomatically make such statements as "human beings cannot rise from the dead", "Socrates was a human being and not an immortal vampire", and "we did not take hallucinogenic drugs which caused us to falsely believe the lights were on", ad infinitum. Absolute certainty of truth or falsehood exists only in the sense of logically constructed abstract concepts such as "it is impossible to draw a triangle with four sides" and "all bachelors are unmarried".)

For those having difficulty with the concept of something being possible but not true, the meaning of these terms may be made more comprehensible by thinking of multiple "possible worlds" (in the sense of Leibniz) or "alternate universes"; something "necessary" is true in all possible worlds, something "possible" is true in at least one possible world.


---


Message-passing semantics
The actor model is about the semantics of message passing.

Unbounded nondeterminism controversy
Arguably, the first concurrent programs were interrupt handlers. During the course of its normal operation a computer needed to be able to receive information from outside (characters from a keyboard, packets from a network, etc). So when the information arrived the execution of the computer was interrupted and special code (called an interrupt handler) was called to put the information in a data buffer where it could be subsequently retrieved.

In the early 1960s, interrupts began to be used to simulate the concurrent execution of several programs on one processor.[17] Having concurrency with shared memory gave rise to the problem of concurrency control. Originally, this problem was conceived as being one of mutual exclusion on a single computer. Edsger Dijkstra developed semaphores and later, between 1971 and 1973,[18] Tony Hoare[19] and Per Brinch Hansen[20] developed monitors to solve the mutual exclusion problem. However, neither of these solutions provided a programming language construct that encapsulated access to shared resources. This encapsulation was later accomplished by the serializer construct ([Hewitt and Atkinson 1977, 1979] and [Atkinson 1980]).

The first models of computation (e.g., Turing machines, Post productions, the lambda calculus, etc.) were based on mathematics and made use of a global state to represent a computational step (later generalized in [McCarthy and Hayes 1969] and [Dijkstra 1976] see Event orderings versus global state). Each computational step was from one global state of the computation to the next global state. The global state approach was continued in automata theory for finite-state machines and push down stack machines, including their nondeterministic versions. Such nondeterministic automata have the property of bounded nondeterminism; that is, if a machine always halts when started in its initial state, then there is a bound on the number of states in which it halts.

Edsger Dijkstra further developed the nondeterministic global state approach. Dijkstra's model gave rise to a controversy concerning unbounded nondeterminism (also called unbounded indeterminacy), a property of concurrency by which the amount of delay in servicing a request can become unbounded as a result of arbitration of contention for shared resources while still guaranteeing that the request will eventually be serviced. Hewitt argued that the actor model should provide the guarantee of service. In Dijkstra's model, although there could be an unbounded amount of time between the execution of sequential instructions on a computer, a (parallel) program that started out in a well defined state could terminate in only a bounded number of states [Dijkstra 1976]. Consequently, his model could not provide the guarantee of service. Dijkstra argued that it was impossible to implement unbounded nondeterminism.

Hewitt argued otherwise: there is no bound that can be placed on how long it takes a computational circuit called an arbiter to settle (see metastability (electronics)).[21] Arbiters are used in computers to deal with the circumstance that computer clocks operate asynchronously with respect to input from outside, e.g., keyboard input, disk access, network input, etc. So it could take an unbounded time for a message sent to a computer to be received and in the meantime the computer could traverse an unbounded number of states.

The actor model features unbounded nondeterminism which was captured in a mathematical model by Will Clinger using domain theory.[2] In the actor model, there is no global state.[dubious – discuss]

Direct communication and asynchrony
Messages in the actor model are not necessarily buffered. This was a sharp break with previous approaches to models of concurrent computation. The lack of buffering caused a great deal of misunderstanding at the time of the development of the actor model and is still a controversial issue. Some researchers argued that the messages are buffered in the "ether" or the "environment". Also, messages in the actor model are simply sent (like packets in IP); there is no requirement for a synchronous handshake with the recipient.

Actor creation plus addresses in messages means variable topology
A natural development of the actor model was to allow addresses in messages. Influenced by packet switched networks [1961 and 1964], Hewitt proposed the development of a new model of concurrent computation in which communications would not have any required fields at all: they could be empty. Of course, if the sender of a communication desired a recipient to have access to addresses which the recipient did not already have, the address would have to be sent in the communication.

For example, an actor might need to send a message to a recipient actor from which it later expects to receive a response, but the response will actually be handled by a third actor component that has been configured to receive and handle the response (for example, a different actor implementing the observer pattern). The original actor could accomplish this by sending a communication that includes the message it wishes to send, along with the address of the third actor that will handle the response. This third actor that will handle the response is called the resumption (sometimes also called a continuation or stack frame). When the recipient actor is ready to send a response, it sends the response message to the resumption actor address that was included in the original communication.

So, the ability of actors to create new actors with which they can exchange communications, along with the ability to include the addresses of other actors in messages, gives actors the ability to create and participate in arbitrarily variable topological relationships with one another, much as the objects in Simula and other object-oriented languages may also be relationally composed into variable topologies of message-exchanging objects.

Inherently concurrent
As opposed to the previous approach based on composing sequential processes, the actor model was developed as an inherently concurrent model. In the actor model sequentiality was a special case that derived from concurrent computation as explained in actor model theory.

No requirement on order of message arrival
icon
This section needs additional citations for verification. Please help improve this article by adding citations to reliable sources in this section. Unsourced material may be challenged and removed. (March 2012) (Learn how and when to remove this message)
Hewitt argued against adding the requirement that messages must arrive in the order in which they are sent to the actor. If output message ordering is desired, then it can be modeled by a queue actor that provides this functionality. Such a queue actor would queue the messages that arrived so that they could be retrieved in FIFO order. So if an actor X sent a message M1 to an actor Y, and later X sent another message M2 to Y, there is no requirement that M1 arrives at Y before M2.

In this respect the actor model mirrors packet switching systems which do not guarantee that packets must be received in the order sent. Not providing the order of delivery guarantee allows packet switching to buffer packets, use multiple paths to send packets, resend damaged packets, and to provide other optimizations.

For more example, actors are allowed to pipeline the processing of messages. What this means is that in the course of processing a message M1, an actor can designate the behavior to be used to process the next message, and then in fact begin processing another message M2 before it has finished processing M1. Just because an actor is allowed to pipeline the processing of messages does not mean that it must pipeline the processing. Whether a message is pipelined is an engineering tradeoff. How would an external observer know whether the processing of a message by an actor has been pipelined? There is no ambiguity in the definition of an actor created by the possibility of pipelining. Of course, it is possible to perform the pipeline optimization incorrectly in some implementations, in which case unexpected behavior may occur.

Locality
Another important characteristic of the actor model is locality.

Locality means that in processing a message, an actor can send messages only to addresses that it receives in the message, addresses that it already had before it received the message, and addresses for actors that it creates while processing the message. (But see Synthesizing addresses of actors.)

Also locality means that there is no simultaneous change in multiple locations. In this way it differs from some other models of concurrency, e.g., the Petri net model in which tokens are simultaneously removed from multiple locations and placed in other locations.

Composing actor systems
The idea of composing actor systems into larger ones is an important aspect of modularity that was developed in Gul Agha's doctoral dissertation,[6] developed later by Gul Agha, Ian Mason, Scott Smith, and Carolyn Talcott.[9]

Behaviors
A key innovation was the introduction of behavior specified as a mathematical function to express what an actor does when it processes a message, including specifying a new behavior to process the next message that arrives. Behaviors provided a mechanism to mathematically model the sharing in concurrency.

Behaviors also freed the actor model from implementation details, e.g., the Smalltalk-72 token stream interpreter. However, the efficient implementation of systems described by the actor model require extensive optimization. See Actor model implementation for details.

Modeling other concurrency systems
Other concurrency systems (e.g., process calculi) can be modeled in the actor model using a two-phase commit protocol.[22]

Computational Representation Theorem
See also: Denotational semantics of the Actor model
There is a Computational Representation Theorem in the actor model for systems which are closed in the sense that they do not receive communications from outside. The mathematical denotation denoted by a closed system 
S
{\displaystyle {\mathtt {S}}} is constructed from an initial behavior 
⊥
S
{\displaystyle \bot _{\mathtt {S}}} and a behavior-approximating function 
p
r
o
g
r
e
s
s
i
o
n
S
.
{\displaystyle \mathbf {progression} _{\mathtt {S}}.} These obtain increasingly better approximations and construct a denotation (meaning) for 
S
{\displaystyle {\mathtt {S}}} as follows [Hewitt 2008; Clinger 1981]:

D
e
n
o
t
e
S
≡
lim
i
→
∞
p
r
o
g
r
e
s
s
i
o
n
S
i
(
⊥
S
)
{\displaystyle \mathbf {Denote} _{\mathtt {S}}\equiv \lim _{i\to \infty }\mathbf {progression} _{{\mathtt {S}}^{i}}(\bot _{\mathtt {S}})}
In this way, 
S
{\displaystyle {\mathtt {S}}} can be mathematically characterized in terms of all its possible behaviors (including those involving unbounded nondeterminism). Although 
D
e
n
o
t
e
S
{\displaystyle \mathbf {Denote} _{\mathtt {S}}} is not an implementation of 
S
{\displaystyle {\mathtt {S}}}, it can be used to prove a generalization of the Church-Turing-Rosser-Kleene thesis [Kleene 1943]:

A consequence of the above theorem is that a finite actor can nondeterministically respond with an uncountable[clarification needed] number of different outputs.

Relationship to logic programming
icon
This section needs additional citations for verification. Please help improve this article by adding citations to reliable sources in this section. Unsourced material may be challenged and removed. (March 2012) (Learn how and when to remove this message)
One of the key motivations for the development of the actor model was to understand and deal with the control structure issues that arose in development of the Planner programming language.[citation needed] Once the actor model was initially defined, an important challenge was to understand the power of the model relative to Robert Kowalski's thesis that "computation can be subsumed by deduction". Hewitt argued that Kowalski's thesis turned out to be false for the concurrent computation in the actor model (see Indeterminacy in concurrent computation).

Nevertheless, attempts were made to extend logic programming to concurrent computation. However, Hewitt and Agha [1991] claimed that the resulting systems were not deductive in the following sense: computational steps of the concurrent logic programming systems do not follow deductively from previous steps (see Indeterminacy in concurrent computation). Recently, logic programming has been integrated into the actor model in a way that maintains logical semantics.[21]


---


Metalogic is the metatheory of logic. Whereas logic studies how logical systems can be used to construct valid and sound arguments, metalogic studies the properties of logical systems.[1] Logic concerns the truths that may be derived using a logical system; metalogic concerns the truths that may be derived about the languages and systems that are used to express truths.[2]

The basic objects of metalogical study are formal languages, formal systems, and their interpretations. The study of interpretation of formal systems is the branch of mathematical logic that is known as model theory, and the study of deductive systems is the branch that is known as proof theory.

Overview
Formal language
Main article: Formal language
A formal language is an organized set of symbols, the symbols of which precisely define it by shape and place. Such a language therefore can be defined without reference to the meanings of its expressions; it can exist before any interpretation is assigned to it—that is, before it has any meaning. First-order logic is expressed in some formal language. A formal grammar determines which symbols and sets of symbols are formulas in a formal language.

A formal language can be formally defined as a set A of strings (finite sequences) on a fixed alphabet α. Some authors, including Rudolf Carnap, define the language as the ordered pair <α, A>.[3] Carnap also requires that each element of α must occur in at least one string in A.

Formation rules
Main article: Formation rule
Formation rules (also called formal grammar) are a precise description of the well-formed formulas of a formal language. They are synonymous with the set of strings over the alphabet of the formal language that constitute well formed formulas. However, it does not describe their semantics (i.e. what they mean).

Formal systems
Main article: Formal system
A formal system (also called a logical calculus, or a logical system) consists of a formal language together with a deductive apparatus (also called a deductive system). The deductive apparatus may consist of a set of transformation rules (also called inference rules) or a set of axioms, or have both. A formal system is used to derive one expression from one or more other expressions.

A formal system can be formally defined as an ordered triple <α,
I
{\displaystyle {\mathcal {I}}},
D
{\displaystyle {\mathcal {D}}}d>, where 
D
{\displaystyle {\mathcal {D}}}d is the relation of direct derivability. This relation is understood in a comprehensive sense such that the primitive sentences of the formal system are taken as directly derivable from the empty set of sentences. Direct derivability is a relation between a sentence and a finite, possibly empty set of sentences. Axioms are so chosen that every first place member of 
D
{\displaystyle {\mathcal {D}}}d is a member of 
I
{\displaystyle {\mathcal {I}}} and every second place member is a finite subset of 
I
{\displaystyle {\mathcal {I}}}.

A formal system can also be defined with only the relation 
D
{\displaystyle {\mathcal {D}}}d. Thereby can be omitted 
I
{\displaystyle {\mathcal {I}}} and α in the definitions of interpreted formal language, and interpreted formal system. However, this method can be more difficult to understand and use.[3]

Formal proofs
Main article: Formal proof
A formal proof is a sequence of well-formed formulas of a formal language, the last of which is a theorem of a formal system. The theorem is a syntactic consequence of all the well formed formulae that precede it in the proof system. For a well formed formula to qualify as part of a proof, it must result from applying a rule of the deductive apparatus of some formal system to the previous well formed formulae in the proof sequence.

Interpretations
Main articles: Interpretation (logic) and Formal semantics (logic)
An interpretation of a formal system is the assignment of meanings to the symbols and truth-values to the sentences of the formal system. The study of interpretations is called Formal semantics. Giving an interpretation is synonymous with constructing a model.

Important distinctions
Metalanguage–object language
Main article: Metalanguage
In metalogic, formal languages are sometimes called object languages. The language used to make statements about an object language is called a metalanguage. This distinction is a key difference between logic and metalogic. While logic deals with proofs in a formal system, expressed in some formal language, metalogic deals with proofs about a formal system which are expressed in a metalanguage about some object language.

Syntax–semantics
Main articles: Syntax (logic) and Formal semantics (logic)
In metalogic, 'syntax' has to do with formal languages or formal systems without regard to any interpretation of them, whereas, 'semantics' has to do with interpretations of formal languages. The term 'syntactic' has a slightly wider scope than 'proof-theoretic', since it may be applied to properties of formal languages without any deductive systems, as well as to formal systems. 'Semantic' is synonymous with 'model-theoretic'.

Use–mention
Main article: Use–mention distinction
In metalogic, the words use and mention, in both their noun and verb forms, take on a technical sense in order to identify an important distinction.[2] The use–mention distinction (sometimes referred to as the words-as-words distinction) is the distinction between using a word (or phrase) and mentioning it. Usually it is indicated that an expression is being mentioned rather than used by enclosing it in quotation marks, printing it in italics, or setting the expression by itself on a line. The enclosing in quotes of an expression gives us the name of an expression, for example:

"Metalogic" is the name of this article.
This article is about metalogic.

Type–token
Main article: Type–token distinction
The type-token distinction is a distinction in metalogic, that separates an abstract concept from the objects which are particular instances of the concept. For example, the particular bicycle in your garage is a token of the type of thing known as "The bicycle." Whereas, the bicycle in your garage is in a particular place at a particular time, that is not true of "the bicycle" as used in the sentence: "The bicycle has become more popular recently." This distinction is used to clarify the meaning of symbols of formal languages.