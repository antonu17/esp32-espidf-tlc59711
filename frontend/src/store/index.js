import Vue from 'vue'
import Vuex from 'vuex'
const axios = require('axios').default;

Vue.use(Vuex)

export default new Vuex.Store({
  state: {
    loadingStatus: 'notLoading',
    effects: [],
    currentEffect: null,
    currentMode: null
  },
  mutations: {
    SET_LOADING_STATUS(state, status) {
      state.loadingStatus = status
    },
    SET_EFFECTS(state, effects) {
      state.effects = effects
    },
    SET_CURRENT_EFFECT(state, effect) {
      state.currentEffect = effect
    },
    SET_CURRENT_MODE(state, mode) {
      state.currentMode = mode
    }
  },
  actions: {
    fetchEffects(context) {
      context.commit('SET_LOADING_STATUS', 'loading')
      axios.get('/api/v1/effects')
        .then(function (response) {
          setTimeout(function () {
            context.commit('SET_EFFECTS', response.data.effects)
          }, 1000)
        })
        .catch(function (error) {
          console.log(error);
        })
    },
    fetchCurrentMode(context) {
      axios.get('/api/v1/mode')
        .then(function (response) {
          context.commit('SET_LOADING_STATUS', 'notLoading');
          context.commit('SET_CURRENT_EFFECT', response.data.effect);
          context.commit('SET_CURRENT_MODE', response.data.mode);
        })
        .catch(function (error) {
          console.log(error);
        })
    }
  },
  getters: {
    effects(state) {
      return state.effects;
    },
    currentEffect(state) {
      return state.currentEffect;
    },
    currentMode(state) {
      return state.currentMode;
    },
    isLoading(state) {
      return state.loadingStatus === 'loading';
    }
  }
})
