import React, { useState, useMemo, useEffect } from 'react';
import { 
  Shield, 
  Globe, 
  Wifi, 
  Cpu, 
  Terminal, 
  Activity, 
  Settings, 
  Zap, 
  Network,
  Binary
} from 'lucide-react';

// --- Symbolic Substrate Constants ---
const BRAILLE_MAP = "⠀⠁⠂⠃⠄⠅⠆⠇⠈⠉⠊⠋⠌⠍⠎⠏⠐⠑⠒⠓⠔⠕⠖⠗⠘⠙⠚⠛⠜⠝⠞⠟⠠⠡⠢⠣⠤⠥⠦⠧⠨⠩⠪⠫⠬⠭⠮⠯⠰⠱⠲⠳⠴⠵⠶⠷⠸⠹⠺⠻⠼⠽⠾⠿";
const AEGEAN_MAP = ["𐄇", "𐄈", "𐄉", "𐄊", "𐄋", "𐄌", "𐄍", "𐄎", "𐄏", "𐄐", "𐄑", "𐄒", "𐄓", "𐄔", "𐄕", "𐄖"];

// --- Core Helper: Hash to Substrate ---
const getSubstrateValue = (bits, type) => {
  if (type === 'braille') {
    // Braille uses 6-8 bits, we'll take 6-bit chunks
    const idx = parseInt(bits.slice(0, 6), 2) % BRAILLE_MAP.length;
    return BRAILLE_MAP[idx];
  }
  if (type === 'aegean') {
    const idx = parseInt(bits.slice(0, 4), 2) % AEGEAN_MAP.length;
    return AEGEAN_MAP[idx];
  }
  return bits.slice(0, 4);
};

// --- Logic Components ---
const StateGrid = ({ bits, type, title }) => {
  const chunks = useMemo(() => {
    const size = type === 'braille' ? 6 : 4;
    const items = [];
    for (let i = 0; i < bits.length; i += size) {
      items.push(bits.slice(i, i + size));
    }
    return items;
  }, [bits, type]);

  return (
    <div className="bg-white/5 p-4 rounded-xl border border-white/10">
      <div className="text-[10px] text-gray-500 font-bold uppercase mb-3 flex justify-between">
        <span>{title}</span>
        <span className="text-indigo-400">{type}</span>
      </div>
      <div className="grid grid-cols-8 gap-1 font-mono text-xl text-center leading-none">
        {chunks.map((chunk, i) => (
          <div key={i} className="hover:text-indigo-400 transition-colors cursor-default" title={chunk}>
            {getSubstrateValue(chunk, type)}
          </div>
        ))}
      </div>
    </div>
  );
};

const App = () => {
  const [substrate, setSubstrate] = useState('braille'); // braille | aegean
  const [stateVector, setStateVector] = useState("1011001110001111000011110011001110101010111100001100110011111111");
  const [networkPrefix, setNetworkPrefix] = useState("0010000000000001000011011011100000000000000000000000000000000001"); // 2001:db8::1
  const [isSyncing, setIsSyncing] = useState(false);

  // Simulated P2P Sync
  useEffect(() => {
    const interval = setInterval(() => {
      if (isSyncing) {
        setStateVector(prev => prev.split('').map(b => Math.random() > 0.95 ? (b === '1' ? '0' : '1') : b).join(''));
      }
    }, 1000);
    return () => clearInterval(interval);
  }, [isSyncing]);

  return (
    <div className="min-h-screen bg-slate-950 text-slate-200 font-sans p-4 md:p-8 selection:bg-indigo-500/30">
      <div className="max-w-6xl mx-auto">
        
        {/* Header */}
        <header className="flex flex-col md:flex-row justify-between items-start md:items-center gap-6 mb-12">
          <div>
            <div className="flex items-center gap-2 text-indigo-400 font-bold text-xs uppercase tracking-[0.2em] mb-2">
              <Shield size={14} /> Universal Logic Witness v4.0
            </div>
            <h1 className="text-4xl font-black tracking-tighter text-white">
              OMICRON <span className="text-indigo-500">PROVER</span>
            </h1>
          </div>

          <div className="flex bg-white/5 p-1 rounded-lg border border-white/10">
            <button 
              onClick={() => setSubstrate('braille')}
              className={`px-4 py-2 rounded-md text-xs font-bold transition-all ${substrate === 'braille' ? 'bg-indigo-600 text-white shadow-lg' : 'text-slate-500 hover:text-slate-300'}`}
            >
              BRAILLE
            </button>
            <button 
              onClick={() => setSubstrate('aegean')}
              className={`px-4 py-2 rounded-md text-xs font-bold transition-all ${substrate === 'aegean' ? 'bg-indigo-600 text-white shadow-lg' : 'text-slate-500 hover:text-slate-300'}`}
            >
              AEGEAN
            </button>
          </div>
        </header>

        {/* Dashboard Grid */}
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-8">
          
          {/* Left: Bipartite State */}
          <div className="lg:col-span-7 space-y-6">
            <section className="bg-slate-900 border border-slate-800 rounded-2xl p-6 shadow-2xl relative overflow-hidden">
              <div className="absolute top-0 right-0 p-4 opacity-10">
                <Network size={120} />
              </div>
              
              <div className="flex justify-between items-center mb-6">
                <h2 className="text-lg font-bold flex items-center gap-2">
                  <Binary className="text-indigo-500" /> Bipartite IPv6 Vector
                </h2>
                <div className="flex items-center gap-2">
                  <span className={`w-2 h-2 rounded-full ${isSyncing ? 'bg-emerald-500 animate-pulse' : 'bg-slate-600'}`} />
                  <span className="text-[10px] font-bold text-slate-500 uppercase tracking-widest">
                    {isSyncing ? 'Active Consensus' : 'Local Stable'}
                  </span>
                </div>
              </div>

              <div className="space-y-4">
                <StateGrid bits={networkPrefix} type={substrate} title="Global Routing (Left 64-bit)" />
                <div className="flex justify-center py-2">
                  <div className="h-8 w-px bg-gradient-to-b from-indigo-500/50 to-transparent" />
                </div>
                <StateGrid bits={stateVector} type={substrate} title="Local State Vector (Right 64-bit)" />
              </div>

              <div className="mt-8 flex gap-4">
                <button 
                  onClick={() => setIsSyncing(!isSyncing)}
                  className={`flex-1 flex items-center justify-center gap-2 py-3 rounded-xl font-bold text-sm transition-all ${
                    isSyncing ? 'bg-red-500/10 text-red-500 border border-red-500/20' : 'bg-indigo-600 text-white'
                  }`}
                >
                  {isSyncing ? <Zap size={16} /> : <Wifi size={16} />}
                  {isSyncing ? 'Stop P2P Sync' : 'Initialize P2P Link'}
                </button>
                <button className="px-4 py-3 rounded-xl bg-slate-800 border border-slate-700 hover:bg-slate-700 transition-colors">
                  <Settings size={18} />
                </button>
              </div>
            </section>

            {/* Metatheorem Feed */}
            <div className="bg-slate-900/50 border border-slate-800 rounded-2xl overflow-hidden">
               <div className="p-4 border-b border-slate-800 bg-slate-800/50 flex justify-between items-center">
                  <span className="text-[10px] font-bold text-slate-500 uppercase tracking-widest">Witness Proofs</span>
                  <Activity size={14} className="text-indigo-500" />
               </div>
               <div className="divide-y divide-slate-800">
                  {[
                    { id: 'Q1', name: 'Peirce Arrow Closure', status: 'Verified', val: '0x3F' },
                    { id: 'Q2', name: 'Bipartite Adjacency', status: 'Verified', val: '0x1A' },
                    { id: 'Q3', name: 'Polynomial Resonance', status: 'Pending', val: '...' },
                  ].map(proof => (
                    <div key={proof.id} className="p-4 flex items-center justify-between hover:bg-white/5 transition-colors">
                      <div className="flex items-center gap-3">
                        <div className="text-xs font-mono bg-indigo-500/10 text-indigo-400 px-2 py-1 rounded">
                          {proof.id}
                        </div>
                        <div className="text-sm font-bold">{proof.name}</div>
                      </div>
                      <div className="text-xs font-mono text-slate-500">{proof.val}</div>
                    </div>
                  ))}
               </div>
            </div>
          </div>

          {/* Right: Technical Readout */}
          <div className="lg:col-span-5 space-y-6">
            <section className="bg-indigo-950/20 border border-indigo-500/20 rounded-2xl p-6 backdrop-blur-xl">
               <h3 className="text-indigo-400 font-bold text-sm uppercase mb-4 flex items-center gap-2">
                 <Terminal size={16} /> Readout Surface
               </h3>
               <div className="font-mono text-[11px] space-y-2 text-indigo-300/70">
                 <p className="">{'>'} [INIT] OMICRON EVALUATOR 4.0</p>
                 <p className="">{'>'} [NET] BIPARTITE ADDR: 2001:DB8::{stateVector.slice(0,4)}:...</p>
                 <p className="">{'>'} [SUB] SUBSTRATE MAPPING: {substrate.toUpperCase()}</p>
                 <p className="text-emerald-400">{'>'} [OK] CONSENSUS LAYER REACHABLE</p>
                 <div className="h-32 bg-black/40 rounded border border-white/5 p-2 overflow-y-auto custom-scrollbar">
                    <p className="text-slate-500 animate-pulse">Running poly-form truth surface analysis...</p>
                    <p className="text-slate-500">Found 16 Boolean functions in F₂[x]</p>
                    <p className="text-slate-500">Mapping state vector to {substrate} substrate...</p>
                    <p className="text-indigo-400">Current Vector Witness: {stateVector.slice(0, 16)}</p>
                 </div>
               </div>
            </section>

            <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6">
              <h3 className="font-bold text-sm mb-4">State Distribution</h3>
              <div className="space-y-4">
                {[
                  { label: 'Network Entropy', val: 78 },
                  { label: 'State Consistency', val: 94 },
                  { label: 'Substrate Fidelity', val: 100 },
                ].map(stat => (
                  <div key={stat.label}>
                    <div className="flex justify-between text-[10px] font-bold uppercase text-slate-500 mb-1">
                      <span>{stat.label}</span>
                      <span>{stat.val}%</span>
                    </div>
                    <div className="w-full h-1 bg-slate-800 rounded-full overflow-hidden">
                      <div 
                        className="h-full bg-indigo-500 transition-all duration-1000" 
                        style={{ width: `${stat.val}%` }} 
                      />
                    </div>
                  </div>
                ))}
              </div>
            </div>

            <div className="p-4 bg-amber-500/5 border border-amber-500/20 rounded-xl">
              <p className="text-[10px] leading-relaxed text-amber-200/60 font-mono italic">
                "The Braille and Aegean substrates are not just decoration; they are the 
                tactile and visual addresses of the universal machine."
              </p>
            </div>
          </div>

        </div>

        {/* Footer */}
        <footer className="mt-12 pt-8 border-t border-slate-800 flex flex-col md:flex-row justify-between items-center gap-4 text-[10px] font-bold text-slate-600 uppercase tracking-[0.2em]">
          <div className="flex items-center gap-4">
            <span>Axiomatic Research</span>
            <span className="text-indigo-500/50">|</span>
            <span>POPL 2026</span>
          </div>
          <div className="flex items-center gap-2">
            <Globe size={12} /> GLOBAL NODE: {stateVector.slice(0, 8)}
          </div>
        </footer>
      </div>

      <style dangerouslySetInnerHTML={{ __html: `
        .custom-scrollbar::-webkit-scrollbar { width: 4px; }
        .custom-scrollbar::-webkit-scrollbar-track { background: rgba(255,255,255,0.05); }
        .custom-scrollbar::-webkit-scrollbar-thumb { background: rgba(99, 102, 241, 0.2); border-radius: 10px; }
        .custom-scrollbar::-webkit-scrollbar-thumb:hover { background: rgba(99, 102, 241, 0.4); }
      `}} />
    </div>
  );
};

export default App;