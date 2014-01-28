#include "particle.h"

Particle::Particle() :
  id(-1), pdgId(0), charge(999.), name(""), role(-1),
  px(0.), py(0.), pz(0.), status(0), e(-1.), m(-1.),
  _mother(-1), _isPrimary(true)
{
}

Particle::Particle(int role_, int pdgId_) :
  id(-1), charge(999.), name(""), role(-1),
  px(0.), py(0.), pz(0.), status(0), e(-1.), m(-1.),
  _mother(-1), _isPrimary(true)
{
  this->role = role_;
  this->pdgId = pdgId_;
  if (this->pdgId!=0) {
    this->M(-1.);
  }
}

Particle::~Particle() {
#ifdef DEBUG
  std::cout << "[Particle::~Particle] [DEBUG] Destructor called" << std::endl;
#endif
  //if (this->_mother!=(Particle*)NULL) delete this->_mother;
}

Particle&
Particle::operator=(const Particle &part_)
{
  this->pdgId = part_.pdgId;
  this->role = part_.role;
  if (this->id==-1) {
    this->id = part_.id;
  }
  this->P(part_.px, part_.py, part_.pz, part_.e);
  this->M(part_.m);

  return *this;
}

bool
Particle::Valid()
{
  if (this->pdgId==0) return false;
  if (this->P()==0. and this->M()==0.) return false;
  return true;
}

std::string
Particle::GetLHEline(bool revert_)
{
  std::stringstream line;

  if (revert_) {
    pz = -pz;
  }

  line << pdgId << "\t";
  line << "1 1 2 0 0" << "\t";
  line << px << "\t";
  line << py << "\t";
  line << pz << "\t";
  line << E() << "\t";
  line << M() << "\t";
  line << "0." << "\t";
  line << "0."; //FIXME iz!!!
  return line.str();
}

bool
Particle::P(double p_[3], double E_=-1.)
{
  if (E_<0.) return this->P(p_[0], p_[1], p_[2]);
  else return this->P(p_[0], p_[1], p_[2], E_);
}

bool
Particle::M(double m_)
{
  double mass;
  if (m_>=0.) this->m = m_;
  else if (this->pdgId!=0) {
    mass = GetMassFromPDGId(this->pdgId);
    if (mass<0.) return false;
    this->m = mass;
  }
  else return false;
  return true;
}

void
Particle::SetMother(Particle* part_)
{
  this->_mother = part_->id;
  this->_isPrimary = false;
#ifdef DEBUG
  std::cout << "[Particle::SetMother] [DEBUG] Particle "
	    << part_->id+1 << " (pdgId=" << part_->pdgId << ") is the new mother of "
	    << this->id+1 << " (pdgId=" << this->pdgId << ")" << std::endl;
#endif
  part_->AddDaughter(this);
};

bool
Particle::AddDaughter(Particle* part_)
{
  std::pair<std::set<int>::iterator,bool> ret;
  ret = this->_daugh.insert(part_->id);
#ifdef DEBUG
  std::set<int>::iterator it;
  std::cout << "[Particle::AddDaughter] [DEBUG] Particle "
	    << this->role << " (pdgId=" << this->pdgId << ") has now "
	    << this->NumDaughters() << " daughter(s) : " << std::endl;
  for (it=this->_daugh.begin(); it!=this->_daugh.end(); it++) {
    std::cout << " * " << *it << std::endl;
  }
#endif

  if (ret.second) {
#ifdef DEBUG
    std::cout << "[Particle::AddDaughter] [DEBUG] Particle " 
	      << part_->role << " (pdgId=" << part_->pdgId << ") is a new daughter of "
	      << this->role << " (pdgId=" << this->pdgId << ")" << std::endl;
#endif
    if (!part_->Primary() && part_->GetMother()!=-1) {
      part_->SetMother(this);
    }
  }

  return ret.second;
}

std::vector<int>
Particle::GetDaughters()
{
  std::vector<int> out;
  std::set<int>::iterator it;
  
  if (this->_daugh.empty()) return out;
  
  out.reserve(this->_daugh.size());
#ifdef DEBUG
  std::cout << "[Particle::GetDaughters] [DEBUG] Reserved " << this->_daugh.size() << " slot(s) for the daughter particle(s)" << std::endl;
#endif
  
  for (it=this->_daugh.begin(); it!=this->_daugh.end(); it++) {
    if (*it==-1) continue;
#ifdef DEBUG
    std::cout << " * " << *it << std::endl;
#endif
    out.push_back(*it);
  }
#ifdef DEBUG
  std::cout << "[Particle::GetDaughters] [DEBUG] Returning a vector containing " << out.size() << " particle(s)" << std::endl;
#endif
  return out;
}
 
void
Particle::Dump()
{
  std::vector<int> daugh;

  if (this->Valid()) {
    std::cout << "[Particle::Dump]"
	      << "\n  Id = " << this->id
	      << "\n  Role = " << this->role
	      << "\n  Status = " << this->status
	      << "\n  PDG id = " << this->pdgId
	      << "\n  P = (" << this->px << ", " << this->py << ", " << this->pz << ") GeV"
	      << "\n  |P| = " << this->P() << " GeV"
	      << "\n  Pt = " << this->Pt() << " GeV"
	      << "\n  E = " << this->E() << " GeV"
	      << "\n  M = " << this->M() << " GeV"
	      << "\n  eta = " << this->Eta()
	      << "\n  Is valid ? " << this->Valid()
	      << "\n  Is primary ? " << this->_isPrimary << std::endl;
    if (!this->Primary()) {
      std::cout << "  Mother = " << this->GetMother() << std::endl;
    }

    daugh = this->GetDaughters();

    std::cout << "  Daughters (" << this->NumDaughters() << ")" << std::endl;
    for (unsigned int i=0; i<this->NumDaughters(); i++) {
      std::cout << "   * Id = " << daugh[i] << std::endl; 
    }
  }
  else {
    std::cout << "[Particle::Dump] ERROR: Particle with role \"" << this->role << "\" is invalid" << std::endl;
  }
}