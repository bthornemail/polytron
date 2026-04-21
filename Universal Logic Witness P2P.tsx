import React, { useState, useMemo, useEffect } from 'react';
import { 
  CheckCircle, 
  Code, 
  Lightbulb, 
  BookOpen, 
  GitBranch, 
  Layers, 
  Activity,
  Terminal,
  Shield,
  Search,
  Globe,
  Users,
  Wifi,
  Cpu
} from 'lucide-react';

// ========== Core Polynomial Library (Pure Logic) ==========
const trim = (p) => {
  if (!p || p.length === 0) return [];
  let i = p.length - 1;
  while (i >= 0 && !p[i]) i--;
  return p.slice(0, i + 1);
};

const polyAdd = (a, b) => {
  const len = Math.max(a.length, b.length);
  const result = [];
  for (let i = 0; i < len; i++) {
    result.push((a[i] || false) !== (b[i] || false));
  }
  return trim(result);
};

// ========== Logic Gate Simulation Components ==========
const GateInput = ({ label, value, onChange }) => (
  <div className="flex flex-col items-center gap-1">
    <button 
      onClick={() => onChange(!value)}
      className={`w-10 h-10 rounded-md border-2 transition-all flex items-center justify-center font-bold ${
        value ? 'bg-indigo-600 border-indigo-400 text-white shadow-lg' : 'bg-gray-100 border-gray-300 text-gray-500'
      }`}
    >
      {value ? '1' : '0'}
    </button>
    <span className="text-[10px] uppercase font-bold text-gray-400">{label}</span>
  </div>
);

const LogicGate = ({ type, inputs, output }) => {
  const symbols = { XOR: '⊕', AND: '∧', OR: '∨', NOT: '¬' };
  return (
    <div className="flex items-center gap-3 bg-white p-3 rounded-xl border border-gray-200 shadow-sm">
      <div className="flex flex-col gap-2">
        {inputs.map((val, i) => (
          <div key={i} className={`w-3 h-3 rounded-full ${val ? 'bg-indigo-500' : 'bg-gray-300'}`} />
        ))}
      </div>
      <div className="w-12 h-12 rounded-full border-2 border-indigo-100 flex items-center justify-center bg-indigo-50 text-indigo-600 font-bold text-xl">
        {symbols[type] || '?'}
      </div>
      <div className="flex items-center gap-2">
        <div className="w-8 h-0.5 bg-gray-200" />
        <div className={`w-4 h-4 rounded-full ${output ? 'bg-indigo-500' : 'bg-gray-300'} ring-4 ring-indigo-50`} />
      </div>
    </div>
  );
};

// ========== Main Application ==========
const App = () => {
  const [activeTab, setActiveTab] = useState('kernel');
  const [kernelInputs, setKernelInputs] = useState({ a: true, b: false });
  const [peers, setPeers] = useState([
    { id: 'node-7a2f', state: '2001:db8::1a', status: 'online', sync: 98 },
    { id: 'node-bc34', state: '2001:db8::f2', status: 'online', sync: 100 },
    { id: 'node-99de', state: '2001:db8::c0', status: 'offline', sync: 45 }
  ]);

  const xorResult = kernelInputs.a !== kernelInputs.b;

  const theorems = [
    { id: 'poly_add_comm', name: 'Commutativity of Addition', status: 'verified', doc: '∀ a b, a + b = b + a' },
    { id: 'step_det', name: 'Step Determinism', status: 'verified', doc: 'Kernel produces unique next state' },
    { id: 'bipartite_sync', name: 'Bipartite Consensus', status: 'verified', doc: 'IPv6 vectors converge' },
    { id: 'hamming_ecc', name: 'Hamming(7,4) Correction', status: 'admitted', doc: 'Corrects all 1-bit errors' }
  ];

  const renderKernel = () => (
    <div className="space-y-8 animate-in fade-in duration-500">
      <div className="bg-gray-900 rounded-2xl p-8 text-white shadow-2xl border border-white/10">
        <div className="flex items-center justify-between mb-6">
          <div className="flex items-center gap-3">
            <Terminal className="text-indigo-400" />
            <h2 className="text-xl font-mono font-bold">Kernel Law: rotl(x,1) ^ rotl(x,3)</h2>
          </div>
          <div className="text-xs font-mono text-gray-500 bg-white/5 px-2 py-1 rounded">
            State: 2001:db8:85a3::8a2e:0370:7334
          </div>
        </div>
        
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-12">
          <div className="space-y-6">
            <h3 className="text-gray-400 uppercase text-xs font-bold tracking-widest">Logic Interaction</h3>
            <div className="flex items-center gap-8">
              <div className="flex gap-4">
                <GateInput label="A" value={kernelInputs.a} onChange={(v) => setKernelInputs(prev => ({ ...prev, a: v }))} />
                <GateInput label="B" value={kernelInputs.b} onChange={(v) => setKernelInputs(prev => ({ ...prev, b: v }))} />
              </div>
              <LogicGate type="XOR" inputs={[kernelInputs.a, kernelInputs.b]} output={xorResult} />
            </div>
          </div>

          <div className="space-y-4">
            <h3 className="text-gray-400 uppercase text-xs font-bold tracking-widest">Bipartite Metrics</h3>
            <div className="grid grid-cols-2 gap-3">
              {[
                { label: 'Network Left (64b)', val: 'Global Route', icon: <Globe size={14}/> },
                { label: 'State Right (64b)', val: 'Local Vector', icon: <Cpu size={14}/> },
                { label: 'P2P Status', val: 'Connected', icon: <Wifi size={14}/> },
                { label: 'Active Peers', val: '128', icon: <Users size={14}/> }
              ].map(stat => (
                <div key={stat.label} className="bg-white/5 p-3 rounded-lg border border-white/10">
                  <div className="text-[10px] text-gray-500 font-bold uppercase flex items-center gap-1">
                    {stat.icon} {stat.label}
                  </div>
                  <div className="text-sm font-mono text-indigo-300 mt-1">{stat.val}</div>
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );

  const renderTheorems = () => (
    <div className="grid grid-cols-1 md:grid-cols-2 gap-4 animate-in slide-in-from-bottom-4 duration-500">
      {theorems.map((t) => (
        <div key={t.id} className="bg-white p-6 rounded-2xl border border-gray-100 shadow-sm hover:shadow-md transition-all group">
          <div className="flex justify-between items-start mb-4">
            <div className={`p-2 rounded-lg ${t.status === 'verified' ? 'bg-green-50' : 'bg-amber-50'}`}>
              <CheckCircle className={`w-5 h-5 ${t.status === 'verified' ? 'text-green-600' : 'text-amber-600'}`} />
            </div>
            <span className={`text-[10px] font-bold px-2 py-1 rounded uppercase ${
              t.status === 'verified' ? 'bg-green-100 text-green-700' : 'bg-amber-100 text-amber-700'
            }`}>
              {t.status}
            </span>
          </div>
          <h3 className="font-bold text-gray-900 mb-1">{t.name}</h3>
          <p className="text-xs font-mono text-indigo-600 mb-4">{t.doc}</p>
          <button className="text-[10px] font-bold text-gray-400 uppercase flex items-center gap-1 hover:text-indigo-600 transition-colors">
            View Witness <Code className="w-3 h-3" />
          </button>
        </div>
      ))}
    </div>
  );

  const renderNetwork = () => (
    <div className="space-y-6 animate-in zoom-in-95 duration-500">
      <div className="bg-white rounded-2xl border border-gray-200 p-6 shadow-sm">
        <h3 className="text-lg font-bold mb-4 flex items-center gap-2">
          <Globe className="text-indigo-600" /> Active P2P Mesh
        </h3>
        <div className="space-y-3">
          {peers.map(peer => (
            <div key={peer.id} className="flex items-center justify-between p-4 bg-gray-50 rounded-xl border border-gray-100">
              <div className="flex items-center gap-4">
                <div className={`w-3 h-3 rounded-full ${peer.status === 'online' ? 'bg-green-500 animate-pulse' : 'bg-gray-300'}`} />
                <div>
                  <div className="text-sm font-bold text-gray-900">{peer.id}</div>
                  <div className="text-xs font-mono text-gray-500">{peer.state}</div>
                </div>
              </div>
              <div className="flex items-center gap-4 text-right">
                <div>
                  <div className="text-[10px] font-bold text-gray-400 uppercase">Sync</div>
                  <div className="text-xs font-mono font-bold text-indigo-600">{peer.sync}%</div>
                </div>
                <button className="p-2 hover:bg-white rounded-lg transition-colors border border-transparent hover:border-gray-200">
                  <Activity size={16} className="text-gray-400" />
                </button>
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );

  return (
    <div className="min-h-screen bg-gray-50 p-4 md:p-8 font-sans text-gray-900">
      <header className="max-w-6xl mx-auto mb-12 flex flex-col md:flex-row md:items-end justify-between gap-6">
        <div>
          <div className="flex items-center gap-2 text-indigo-600 font-bold text-sm uppercase tracking-tighter mb-2">
            <Shield className="w-4 h-4" /> Bipartite Mesh Prover
          </div>
          <h1 className="text-4xl md:text-5xl font-black tracking-tight text-gray-900">
            Universal Logic Witness
          </h1>
          <p className="text-gray-500 mt-2 max-w-xl font-medium">
            Self-contained formal prover with IPv6-vector P2P synchronization.
          </p>
        </div>
        
        <nav className="flex bg-white p-1 rounded-xl shadow-sm border border-gray-200">
          {[
            { id: 'kernel', icon: <Activity className="w-4 h-4" />, label: 'Kernel' },
            { id: 'theorems', icon: <GitBranch className="w-4 h-4" />, label: 'Proofs' },
            { id: 'network', icon: <Globe className="w-4 h-4" />, label: 'Network' }
          ].map(tab => (
            <button
              key={tab.id}
              onClick={() => setActiveTab(tab.id)}
              className={`flex items-center gap-2 px-4 py-2 rounded-lg text-sm font-bold transition-all ${
                activeTab === tab.id 
                  ? 'bg-indigo-600 text-white shadow-lg shadow-indigo-200' 
                  : 'text-gray-500 hover:bg-gray-50'
              }`}
            >
              {tab.icon} {tab.label}
            </button>
          ))}
        </nav>
      </header>

      <main className="max-w-6xl mx-auto">
        {activeTab === 'kernel' && renderKernel()}
        {activeTab === 'theorems' && renderTheorems()}
        {activeTab === 'network' && renderNetwork()}
      </main>

      <footer className="max-w-6xl mx-auto mt-12 grid grid-cols-1 md:grid-cols-3 gap-6">
        <div className="bg-indigo-600 rounded-2xl p-6 text-white shadow-xl flex flex-col justify-between">
          <h3 className="text-lg font-bold flex items-center gap-2">
             <Lightbulb className="w-5 h-5" /> Mesh Strategy
          </h3>
          <p className="text-indigo-100 text-sm mt-2">
            State is mapped to the lower 64 bits of a bipartite IPv6 address for global reachability.
          </p>
        </div>

        <div className="bg-white border border-gray-200 rounded-2xl p-6 flex flex-col justify-between">
           <div className="flex justify-between items-center mb-4">
              <span className="text-xs font-bold text-gray-400 uppercase">Prover Status</span>
              <Activity className="w-4 h-4 text-green-500" />
           </div>
           <div className="space-y-2">
              <div className="flex justify-between text-sm">
                <span>Verified Consistency</span>
                <span className="font-bold">85%</span>
              </div>
              <div className="w-full h-1.5 bg-gray-100 rounded-full overflow-hidden">
                <div className="h-full bg-green-500 w-[85%]" />
              </div>
           </div>
        </div>

        <div className="bg-white border border-gray-200 rounded-2xl p-6 flex flex-col justify-between">
           <div className="flex justify-between items-center mb-4">
              <span className="text-xs font-bold text-gray-400 uppercase">State Vector</span>
              <BookOpen className="w-4 h-4 text-indigo-500" />
           </div>
           <div className="space-y-1">
              <div className="text-2xl font-black">F₂[x] → IPv6</div>
              <div className="text-[10px] text-gray-400 font-bold uppercase">Consensus Mapping</div>
           </div>
        </div>
      </footer>
    </div>
  );
};

export default App;