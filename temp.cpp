template <class Archive>
inline void serialize(Archive &ar, VectorRecord &e const unsigned int version) {
  ar &(this->sequence_number);
  ar &(this->member_id);
}

template <class Archive>
inline void serialize(Archive &ar, VectorClock &e const unsigned int version) {
  ar &(this->current_idx);
  ar &(this->records);
}


class VectorRecord {
  uint32_t sequence_number;
  uint64_t member_id;
};

class VectorClock {
  uint16_t current_idx;
  vector<VectorRecord> records;
};
}