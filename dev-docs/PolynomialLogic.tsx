import React, { useState, useMemo, useEffect } from 'react';
import { Box, Hexagon, Circle, Square, Triangle, Shield, Key, Database, Activity, Compass } from 'lucide-react';

const PolytopeLogicApp = () => {
  const [state, setState] = useState(0x1234);
  const [constant, setConstant] = useState(0x0049);
  const [activePolytope, setActivePolytope] = useState('dodecahedron');

  // Kernel Logic
  const rotl = (x, n) => ((x << n) | (x >>> (16 - n))) & 0xFFFF;
  const rotr = (x, n) => ((x >>> n) | (x << (16 - n))) & 0xFFFF;
  const kernel = (x, c) => (rotl(x, 1) ^ rotl(x, 3) ^ rotr(x, 2) ^ c) & 0xFFFF;

  const currentKernel = useMemo(() => kernel(state, constant), [state, constant]);

  // Polytope Definitions as Logic Programs
  const polytopes = {
    tetrahedron: {
      name: "Tetrahedron",
      v: 4, e: 6, f: 4,
      facts: ["Observe", "Activate", "Integrate", "Propagate"],
      rules: [["0", "1"], ["0", "2"], ["0", "3"], ["1", "2"], ["1", "3"], ["2", "3"]],
      clauses: [[0, 1, 2], [0, 1, 3], [0, 2, 3], [1, 2, 3]],
      icon: <Triangle className="w-8 h-8 text-red-400" />
    },
    cube: {
      name: "Cube",
      v: 8, e: 12, f: 6,
      facts: ["Query", "Transform", "Store", "Verify", "Share", "Learn", "Adapt", "Emerge"],
      rules: [
        ["0", "1"], ["1", "2"], ["2", "3"], ["3", "0"],
        ["4", "5"], ["5", "6"], ["6", "7"], ["7", "4"],
        ["0", "4"], ["1", "5"], ["2", "6"], ["3", "7"]
      ],
      clauses: [[0, 1, 2, 3], [4, 5, 6, 7], [0, 1, 5, 4], [1, 2, 6, 5], [2, 3, 7, 6], [3, 0, 4, 7]],
      icon: <Square className="w-8 h-8 text-blue-400" />
    },
    dodecahedron: {
      name: "Dodecahedron",
      v: 20, e: 30, f: 12,
      facts: [
        "Quasar", "Ephemeral", "Catalyst", "Nexus", "Aether", 
        "Mycelium", "Resonance", "Fractal", "Tessellate", "Sonder",
        "Entropy", "Syntropy", "Holonomy", "Isotope", "Flux",
        "Lattice", "Prism", "Vortex", "Horizon", "Singularity"
      ],
      // Subset of logic for visual brevity
      rules: [["0", "1"], ["1", "2"], ["2", "3"], ["3", "4"], ["4", "0"]],
      clauses: [[0, 1, 2, 3, 4]],
      icon: <Hexagon className="w-8 h-8 text-purple-400" />
    }
  };

  const poly = polytopes[activePolytope];

  return (
    <div className="min-h-screen bg-[#0a0a0c] text-slate-300 p-4 md:p-8 font-mono">
      <div className="max-w-6xl mx-auto space-y-8">
        
        {/* Header - The Centroid Origin */}
        <div className="flex flex-col md:flex-row justify-between items-start border-b border-slate-800 pb-6 gap-4">
          <div>
            <h1 className="text-3xl font-bold text-white flex items-center gap-3">
              <Shield className="text-blue-500" /> 
              Polytope Logic Engine
            </h1>
            <p className="text-slate-500 mt-2">Euler Logic: V - E + F = 2 | Facts - Rules + Clauses = Invariant</p>
          </div>
          <div className="bg-slate-900 p-4 rounded-xl border border-blue-500/30 flex items-center gap-4">
            <div className="text-center">
              <div className="text-[0.6rem] text-blue-400 uppercase">Centroid (Private Key)</div>
              <div className="text-xl font-bold text-white">f(x) = 0</div>
            </div>
            <div className="h-8 w-[1px] bg-slate-700" />
            <div className="text-center">
              <div className="text-[0.6rem] text-green-400 uppercase">Kernel Projection</div>
              <div className="text-xl font-bold text-green-400">0x{currentKernel.toString(16).toUpperCase()}</div>
            </div>
          </div>
        </div>

        {/* Input Vectors */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
          <div className="bg-[#16161e] p-6 rounded-2xl border border-slate-800">
            <label className="block text-xs font-bold text-slate-500 mb-4 uppercase">State (X) - typed variable placeholder</label>
            <input 
              type="range" min="0" max="65535" value={state} 
              onChange={(e) => setState(parseInt(e.target.value))}
              className="w-full h-2 bg-slate-800 rounded-lg appearance-none cursor-pointer accent-blue-500"
            />
            <div className="mt-2 text-blue-400 text-right">0x{state.toString(16).toUpperCase()}</div>
          </div>
          <div className="bg-[#16161e] p-6 rounded-2xl border border-slate-800">
            <label className="block text-xs font-bold text-slate-500 mb-4 uppercase">Constant (C) - transformation seed</label>
            <input 
              type="range" min="0" max="65535" value={constant} 
              onChange={(e) => setConstant(parseInt(e.target.value))}
              className="w-full h-2 bg-slate-800 rounded-lg appearance-none cursor-pointer accent-orange-500"
            />
            <div className="mt-2 text-orange-400 text-right">0x{constant.toString(16).toUpperCase()}</div>
          </div>
        </div>

        {/* Polytope Selection */}
        <div className="flex gap-4 overflow-x-auto pb-2">
          {Object.keys(polytopes).map(key => (
            <button
              key={key}
              onClick={() => setActivePolytope(key)}
              className={`flex-1 min-w-[150px] p-4 rounded-xl border transition-all flex flex-col items-center gap-2 ${
                activePolytope === key ? 'border-blue-500 bg-blue-500/10' : 'border-slate-800 bg-black'
              }`}
            >
              {polytopes[key].icon}
              <span className="text-sm font-bold uppercase">{polytopes[key].name}</span>
              <span className="text-[0.6rem] opacity-50">V:{polytopes[key].v} E:{polytopes[key].e} F:{polytopes[key].f}</span>
            </button>
          ))}
        </div>

        {/* The Logic Program Projection */}
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          
          {/* VERTICES: THE FACTS */}
          <div className="bg-black/40 border border-slate-800 rounded-2xl p-6">
            <h3 className="text-blue-400 text-xs font-bold mb-4 flex items-center gap-2">
              <Database size={14} /> 0D: THE FACTS (Atoms)
            </h3>
            <div className="grid grid-cols-2 gap-2">
              {poly.facts.map((fact, i) => (
                <div 
                  key={i} 
                  className={`p-2 rounded border text-[0.7rem] transition-colors ${
                    (currentKernel % poly.v) === i ? 'bg-blue-600 border-white text-white font-bold' : 'bg-slate-900 border-slate-800 text-slate-400'
                  }`}
                >
                  {i}: {fact}
                </div>
              ))}
            </div>
          </div>

          {/* EDGES: THE RULES */}
          <div className="bg-black/40 border border-slate-800 rounded-2xl p-6">
            <h3 className="text-green-400 text-xs font-bold mb-4 flex items-center gap-2">
              <Activity size={14} /> 1D: THE RULES (Implications)
            </h3>
            <div className="space-y-2 max-h-[300px] overflow-y-auto pr-2 custom-scrollbar">
              {poly.rules.map((rule, i) => (
                <div key={i} className="text-[0.65rem] p-2 bg-slate-900 rounded border border-slate-800 flex justify-between">
                  <span className="text-slate-500">rule_{i}(X, Y) :-</span>
                  <span className="text-green-400">{poly.facts[rule[0]]} → {poly.facts[rule[1]]}</span>
                </div>
              ))}
              <div className="pt-4 border-t border-slate-800 text-center text-[0.6rem] text-slate-500 italic">
                Total implications: {poly.e}
              </div>
            </div>
          </div>

          {/* FACES: THE CLAUSES */}
          <div className="bg-black/40 border border-slate-800 rounded-2xl p-6">
            <h3 className="text-purple-400 text-xs font-bold mb-4 flex items-center gap-2">
              <Compass size={14} /> 2D: THE CLAUSES (Conjunctions)
            </h3>
            <div className="space-y-4">
              {poly.clauses.map((clause, i) => (
                <div key={i} className="p-3 bg-purple-500/5 border border-purple-500/20 rounded-lg">
                  <div className="text-[0.6rem] text-purple-400 mb-1 font-bold">CLAUSE_{i} (Planar Group)</div>
                  <div className="text-[0.7rem] leading-relaxed">
                    {clause.map(vIdx => poly.facts[vIdx]).join(' ∧ ')}
                  </div>
                </div>
              ))}
              <div className="p-4 bg-slate-900 rounded-xl border border-slate-800">
                <div className="text-[0.5rem] uppercase text-slate-500 mb-2">Polynomial Mapping</div>
                <div className="text-[0.7rem] text-orange-400 italic">
                  P(x) = Σ (vᵢ * xⁱ) mod f(x)=0
                </div>
              </div>
            </div>
          </div>

        </div>

        {/* Footer */}
        <div className="flex justify-between items-center text-[0.6rem] text-slate-600 border-t border-slate-900 pt-4 italic">
          <div>Shared Centroid Identity Verified [H(f(x)=0)]</div>
          <div>Typed Variable System v1.0 - Church Encoded Lambda Topology</div>
        </div>
      </div>
      
      <style dangerouslySetInnerHTML={{ __html: `
        .custom-scrollbar::-webkit-scrollbar { width: 4px; }
        .custom-scrollbar::-webkit-scrollbar-track { background: transparent; }
        .custom-scrollbar::-webkit-scrollbar-thumb { background: #334155; border-radius: 10px; }
      `}} />
    </div>
  );
};

export default PolytopeLogicApp;